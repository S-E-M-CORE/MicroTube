
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"


#include "oatpp/web/mime/multipart/TemporaryFileProvider.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"
#include "oatpp/web/mime/multipart/PartReader.hpp"
#include "dto/UserDTO.hpp"
#include "dto/InfoJsonDTO.hpp"
#include "dto/VideoListDTO.hpp"
#include "../videos/VideoIndexer.hpp"


#include "../sql/DatabaseClient.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <random> 
#include <sstream>
#include <iomanip>
#include <string> 
#include <cstdint>
#include <chrono>


namespace multipart = oatpp::web::mime::multipart;
namespace dto = microTube::dto;

using VideoIndexer = microTube::video::VideoIndexer;

namespace microTube
{
    namespace apicontroller
    {

        class VideoController : public oatpp::web::server::api::ApiController
        {

        public:
            typedef VideoController __ControllerType;

        public:
            VideoController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                : oatpp::web::server::api::ApiController(objectMapper)
            {
            }

        public:
            static std::shared_ptr<VideoController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
            {
                return std::make_shared<VideoController>(objectMapper);
            }

#include OATPP_CODEGEN_BEGIN(ApiController)

            ENDPOINT_INFO(GetVideo)
            {
                info->summary = "Stream video";
                info->description = "Stream a video file for a given video UUID with support for byte-range requests.";

                info->queryParams.add<String>("uuid").description = "Unique identifier of the video.";

                info->addResponse<oatpp::String>(Status::CODE_206, "video/mp4").description = "Partial content of the video file for streaming.";
                info->addResponse<oatpp::String>(Status::CODE_400, "text/plain").description = "Bad Request: Missing or invalid 'uuid' query parameter.";
                info->addResponse<oatpp::String>(Status::CODE_404, "text/plain").description = "Not Found: Video file not found.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/stream", GetVideo)
            {
                ENDPOINT_ASYNC_INIT(GetVideo);
            private:
                String m_uuid;
            public:
                Action act() override
                {
                    constexpr char* VideoFileName = "video.mp4";
                    constexpr std::size_t MAX_CHUNK_SIZE = 1024 * 1024 * 10; // 10 * 1 MB

                    m_uuid = request->getQueryParameter("uuid");
                    OATPP_ASSERT_HTTP(m_uuid && !m_uuid->empty(), Status::CODE_400, "UUID required");

                    addVideoToWatched();

                    oatpp::String rangeHeader = request->getHeader(Header::RANGE);

                    if (!rangeHeader)
                        rangeHeader = "bytes=0-" + oatpp::utils::Conversion::uint64ToStr(MAX_CHUNK_SIZE);

                    std::filesystem::create_directory(VIDEO_FOLDER);
                    const std::string& FullFilePath = std::string(VIDEO_FOLDER) + "/" + *m_uuid + "/" + std::string(VideoFileName);

                    OATPP_ASSERT_HTTP(std::filesystem::exists(FullFilePath.c_str()), Status::CODE_404, "Not found");

                    const std::int64_t FileSize = static_cast<int64_t>(getFileSize(FullFilePath));

                    oatpp::web::protocol::http::Range range = oatpp::web::protocol::http::Range::parse(rangeHeader);

                    if (range.end == 0)
                        range.end = std::min(range.start + MAX_CHUNK_SIZE, static_cast<size_t>(FileSize));

                    if (range.start > range.end)
                        range.start = range.end - MAX_CHUNK_SIZE;

                    std::streampos start = range.start;
                    std::streamsize length = range.end - range.start;

                    if (start + length > range.end)
                        length = range.end;

                    std::vector<char> buffer(length);

                    std::ifstream file(FullFilePath, std::ios::binary);

                    if (!file.is_open())
                        return _return(controller->createResponse(Status::CODE_404, "File not found"));

                    file.seekg(start);
                    file.read(buffer.data(), length);

                    file.close();

                    const auto& RetPart = std::string(buffer.begin(), buffer.end());

                    auto response = controller->createResponse(Status::CODE_206, RetPart);

                    response->putHeaderIfNotExists("Content-Type", "video/mp4");
                    response->putHeaderIfNotExists("Content-Length", oatpp::utils::Conversion::uint64ToStr(range.end - range.start));
                    response->putHeaderIfNotExists("Content-Range", std::string("bytes ") + oatpp::utils::Conversion::uint64ToStr(range.start) + "-" + oatpp::utils::Conversion::uint64ToStr(range.end - 1) + "/" + oatpp::utils::Conversion::uint64ToStr(FileSize));
                    response->putHeaderIfNotExists("Accept-Ranges", "bytes");
                    response->putHeaderIfNotExists("Connection", "keep-alive");
                    response->putHeaderIfNotExists("Cache-Control", "private, no-cache, no-store, must-revalidate");

                    return _return(std::move(response));
                }

                void addVideoToWatched()
                {
                    const auto& TokenIdPair = controller->m_database->isBearerTokenValid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                    if (TokenIdPair)
                        controller->m_database->addUniqueView(this->m_uuid, TokenIdPair->id);
                }

                static size_t getFileSize(const std::string & filename)
                {
                    try
                    {
                        return std::filesystem::file_size(filename);
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        OATPP_LOGe("VideoController", e.what());
                        return -1;
                    }
                }
            };

            ENDPOINT_INFO(uploadVideo)
            {
                info->summary = "Upload video";
                info->description = "Upload a video file using multipart and associate it with the user's account.";

                info->addResponse<String>(Status::CODE_200, "text/plain").description = "Successfully uploaded the video. Returns the unique identifier (UUID) of the video.";
                info->addResponse<String>(Status::CODE_400, "text/plain").description = "Bad Request: The video file is missing or empty.";
                info->addResponse<String>(Status::CODE_500, "text/plain").description = "Internal Server Error: An unexpected error occurred during processing.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("POST", "api/v1/video/upload", uploadVideo)
            {
                ENDPOINT_ASYNC_INIT(uploadVideo)

                    Action act() override
                {
                    std::filesystem::create_directory(TEMP_FOLDER);
                    m_uploader = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));
                    m_multipart = std::make_shared < multipart::PartList> (request->getHeaders());
                    m_multipartReader = std::make_shared<multipart::AsyncReader>(m_multipart);

                    m_multipartReader->setDefaultPartReader(multipart::createAsyncTemporaryFilePartReader(TEMP_FOLDER, 10, 10000));

                    return request->transferBodyAsync(m_multipartReader).next(yieldTo(&uploadVideo::onUploaded));
                }

                Action onUploaded()
                {
                    const String& FullVideo = this->assembleMultiparts();
                    OATPP_ASSERT_HTTP(FullVideo || !FullVideo.operator std::string().empty(), Status::CODE_400, "Video is empty");

                    const std::string& VideoUUID = this->generateUUID();

                    auto InfoJsonDto        = microTube::dto::InfoJsonDTO::createShared();
                    InfoJsonDto->userId     = m_uploader->id.getPtr();
                    InfoJsonDto->channelId  = this->getUserChannelId();
                    InfoJsonDto->uuid       = VideoUUID;
                    InfoJsonDto->uploadDate = this->getCurrentDate();

                    const std::string& VideoFolderPath = std::string(VIDEO_FOLDER) + "/" + VideoUUID;

                    std::filesystem::create_directory(VIDEO_FOLDER);
                    std::filesystem::create_directory(VideoFolderPath);

                    controller->videoIndexer->updateVideoInfo(InfoJsonDto, true);

                    FullVideo.saveToFile((VideoFolderPath + "/video.mp4").c_str());

                    return _return(controller->createResponse(Status::CODE_200, VideoUUID.c_str()));
                }

                UInt64 getUserChannelId()
                {
                    UInt64 channelId = m_uploader->id;

                    if (!channelId)
                    {
                        auto dbResult = controller->m_database->createNewChannel("Hallo! Ich verwende MicroTube!");
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        const auto newChannelIdList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!newChannelIdList->empty() && newChannelIdList->size() == 1, Status::CODE_500, "Internal server error");

                        channelId = newChannelIdList[0][0];

                        OATPP_ASSERT_HTTP(channelId, Status::CODE_500, "Internal Server Error");

                        dbResult = controller->m_database->createUserChannelJunction(channelId, m_uploader->id);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    }

                    return channelId;
                }

                String assembleMultiparts()
                {
                    oatpp::String videoData;

                    for (auto& part : m_multipart->getAllParts())
                        if (!videoData || videoData->empty())
                            videoData = oatpp::String::loadFromFile(part.get()->getPayload()->getLocation()->c_str());
                        else
                            videoData->append(oatpp::String::loadFromFile(part.get()->getPayload()->getLocation()->c_str()));

                    return std::move(videoData);
                }

                static std::string generateUUID()
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

                    uint32_t part1 = dist(gen);
                    uint16_t part2 = static_cast<uint16_t>(dist(gen) >> 16);
                    uint16_t part3 = static_cast<uint16_t>(dist(gen) >> 16);

                    part3 = (part3 & 0x0FFF) | 0x4000;

                    uint16_t part4 = static_cast<uint16_t>(dist(gen) >> 16);

                    part4 = (part4 & 0x3FFF) | 0x8000;

                    uint64_t part5 = (static_cast<uint64_t>(dist(gen)) << 32) | dist(gen);

                    std::stringstream ss;
                    ss << std::hex << std::setfill('0') << std::nouppercase
                        << std::setw(8) << part1 << '-'
                        << std::setw(4) << part2 << '-'
                        << std::setw(4) << part3 << '-'
                        << std::setw(4) << part4 << '-'
                        << std::setw(12) << part5;

                    return ss.str();
                }

                static std::string getCurrentDate()
                {
                    auto now = std::chrono::system_clock::now();
                    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

                    std::tm localTime = *std::localtime(&currentTime);

                    std::ostringstream oss;
                    oss << std::put_time(&localTime, "%Y-%m-%d");
                    return oss.str();
                }

            private:
                std::shared_ptr<multipart::PartList>                m_multipart;
                std::shared_ptr<multipart::AsyncReader>             m_multipartReader;
                dto::UserTokenPairDTO::Wrapper                      m_uploader;
            };

            ENDPOINT_INFO(GetVideoInfo)
            {
                info->summary = "Get video information";
                info->description = "Retrieve detailed information about a video, including metadata and statistics.";

                info->queryParams.add<String>("uuid").description = "Unique identifier of the video.";

                info->addResponse<Object<dto::VideoInfoTransferDTO>>(Status::CODE_200, "application/json").description = "Successfully retrieved video information.";
                info->addResponse<String>(Status::CODE_400, "text/plain").description = "Bad Request: Missing or invalid 'uuid' query parameter.";
                info->addResponse<String>(Status::CODE_404, "text/plain").description = "Not Found: Video or information file not found.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/info", GetVideoInfo)
            {
                ENDPOINT_ASYNC_INIT(GetVideoInfo);

                Action act() override
                {
                    m_uuid = request->getQueryParameter("uuid");
                    OATPP_ASSERT_HTTP(m_uuid && !m_uuid->empty(), Status::CODE_400, "UUID required");
                    return this->retrieveInfo();
                }

                Action retrieveInfo()
                {
                    constexpr char* InfoJsonFileName = "info.json";

                    const std::string& VideoFolderPath = std::string(VIDEO_FOLDER) + "/" + m_uuid;
                    const std::string& InfoJsonFilePath = VideoFolderPath + "/" + InfoJsonFileName;

                    OATPP_ASSERT_HTTP(std::filesystem::exists(VideoFolderPath), Status::CODE_404, "Video not found");
                    OATPP_ASSERT_HTTP(std::filesystem::exists(InfoJsonFilePath), Status::CODE_404, "Info not found");

                    auto JsonFile = oatpp::String::loadFromFile(InfoJsonFilePath.c_str());
                    OATPP_ASSERT_HTTP(JsonFile && !JsonFile->empty(), Status::CODE_404, "Info not retrievable");

                    m_VideoInfoDTO = dto::VideoInfoTransferDTO::createShared();

                    m_VideoInfoDTO->jsonData = controller->m_objectMapper->readFromString<Object<dto::InfoJsonDTO>>(JsonFile);

                    OATPP_ASSERT_HTTP(m_VideoInfoDTO->jsonData, Status::CODE_404, "Info not readable");

                    m_VideoInfoDTO->likeCount = controller->videoIndexer->getLikeCountFor(m_uuid);
                    m_VideoInfoDTO->dislikeCount = controller->videoIndexer->getDisikeCountFor(m_uuid);
                    m_VideoInfoDTO->uniqueViews = controller->videoIndexer->getUniqueViewCountFor(m_uuid);

                    return _return(controller->createDtoResponse(Status::CODE_200, m_VideoInfoDTO));
                }

            private:
                oatpp::String m_uuid;
                oatpp::Object<dto::VideoInfoTransferDTO> m_VideoInfoDTO;

            };

            ENDPOINT_INFO(UpdateVideoInfo)
            {
                info->summary = "Update video information";
                info->description = "This endpoint allows updating the metadata of a video, such as title, description, upload date, categories, and tags. The request requires the video UUID as a query parameter and a JSON body with the updated video details.";

                info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                info->headers.add<String>("uuid").description = "The UUID of the video to update. This should be passed as a query parameter.";

                // Consuming InfoJsonDTO as the request body
                info->addConsumes<Object<dto::InfoJsonDTO>>("application/json", "Updated video information in JSON format.");

                // Creating an example for the request body
                auto example = dto::InfoJsonDTO::createShared();
                example->uuid = "12345-abcde-67890";
                example->title = "My Updated Video Title";
                example->description = "A detailed description of the video content";
                example->uploadDate = "2024-12-29";
                example->userId = 1001;
                example->channelId = 2002;
                example->categories = { "Technology", "Education" };
                example->tags = { "tutorial", "tech", "2024" };

                info->body.addExample("Updating video information", example);

                // Response descriptions
                info->addResponse<String>(Status::CODE_200, "text/plain")
                    .description = "Successfully updated the video information.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Missing or invalid required fields in the request body.";

                info->addResponse<String>(Status::CODE_401, "text/plain")
                    .description = "Unauthorized: The user is not authorized to update this video.";

                info->addResponse<String>(Status::CODE_404, "text/plain")
                    .description = "Not Found: The video or video information could not be found.";

                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An unexpected error occurred while updating the video information.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("POST", "api/v1/video/info", UpdateVideoInfo)
            {
                ENDPOINT_ASYNC_INIT(UpdateVideoInfo);

                Action act() override
                {
                    m_requester = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                    m_uuid = request->getQueryParameter("uuid");
                    OATPP_ASSERT_HTTP(m_uuid && !m_uuid->empty(), Status::CODE_400, "UUID required");

                    return request->readBodyToDtoAsync<Object<dto::InfoJsonDTO>>(controller->m_objectMapper).callbackTo(&UpdateVideoInfo::onBodyRead);
                }

                Action onBodyRead(const Object<dto::InfoJsonDTO>& dto)
                {
                    constexpr char* InfoJsonFileName = "info.json";

                    m_requester = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                    const std::string& VideoFolderPath = std::string(VIDEO_FOLDER) + "/" + m_uuid;
                    const std::string& InfoJsonFilePath = VideoFolderPath + "/" + InfoJsonFileName;

                    OATPP_ASSERT_HTTP(std::filesystem::exists(VideoFolderPath) , Status::CODE_404, "Video not found");
                    OATPP_ASSERT_HTTP(std::filesystem::exists(InfoJsonFilePath), Status::CODE_500, "Info not found");

                    m_currentJsonFile = oatpp::String::loadFromFile(InfoJsonFilePath.c_str());

                    const auto& currentJsonDto = controller->m_objectMapper->readFromString<Object<dto::InfoJsonDTO>>(m_currentJsonFile);

                    if (dto->userId != m_requester->id)
                    {
                        UInt64 UserChannelId = getUserChannelId();

                        if (!UserChannelId || UserChannelId != currentJsonDto->channelId)
                            OATPP_ASSERT_HTTP(controller->m_database->isUserAdmin(m_requester->id), Status::CODE_401, "Unauthorized");
                    }

                    Object<dto::InfoJsonDTO> updateDto = dto;
                    updateDto->uuid = currentJsonDto->uuid;

                    controller->videoIndexer->updateVideoInfo(dto);

                    return _return(controller->createResponse(Status::CODE_200));
                }

                UInt64 getUserChannelId()
                {
                    auto dbResult = controller->m_database->getChannelIdByUserId(m_requester->id);

                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal Server Error");

                    auto resultList = dbResult->fetch<List<List<UInt64>>>();

                    OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal Server Error");

                    if (resultList->empty())
                        return UInt64();
                    else
                        return resultList[0][0];
                }

            private:
                Object<dto::UserTokenPairDTO> m_requester;
                String                        m_uuid;
                String                        m_currentJsonFile;
            };

            ENDPOINT_INFO(GetThumbnail)
            {
                info->summary = "Get video thumbnail";
                info->description = "Retrieve the thumbnail for the given video UUID.";
                info->queryParams.add<String>("uuid").description = "Unique identifier of the video";

                info->addResponse<oatpp::String>(Status::CODE_200, "image/jpeg").description = "Successfully retrieved the video thumbnail";
                info->addResponse<oatpp::String>(Status::CODE_400, "text/plain").description = "Bad Request: Missing or invalid 'uuid' query parameter";
                info->addResponse<oatpp::String>(Status::CODE_404, "text/plain").description = "Not Found: Thumbnail not found for the given UUID";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/thumbnail", GetThumbnail)
            {
                ENDPOINT_ASYNC_INIT(GetThumbnail);

                Action act() override
                {
                    oatpp::String videoUUID = request->getQueryParameter("uuid");
                    OATPP_ASSERT_HTTP(!videoUUID->empty(), Status::CODE_400, "UUID is empty");

                    oatpp::String thumbnailPath = VIDEO_FOLDER;
                    thumbnailPath = thumbnailPath + "/" + videoUUID + "/thumbnail.jpg";

                    OATPP_ASSERT_HTTP(std::filesystem::exists(thumbnailPath->c_str()), Status::CODE_404, "Thumbnail not found");

                    auto fileData = oatpp::String::loadFromFile(thumbnailPath->c_str());
                    return _return(controller->createResponse(Status::CODE_200, fileData));
                }
            };

            ENDPOINT_INFO(UploadThumbnail)
            {
                info->summary = "Upload video thumbnail";
                info->description = "Uploads a thumbnail image for a specific video identified by its UUID.";

                info->queryParams.add<String>("uuid").description = "Unique identifier of the video to upload the thumbnail for.";
                info->addConsumes<String>("image/jpeg").description = "The JPEG image file to be used as the thumbnail.";

                info->addResponse<String>(Status::CODE_200, "text/plain").description = "Thumbnail uploaded successfully.";
                info->addResponse<String>(Status::CODE_400, "text/plain").description = "Bad Request: Missing or invalid 'uuid' query parameter, or request body is empty.";
                info->addResponse<String>(Status::CODE_404, "text/plain").description = "Not Found: Video or its metadata could not be located.";
                info->addResponse<String>(Status::CODE_415, "text/plain").description = "Unsupported Media Type: File type is not 'image/jpeg'.";
                info->addResponse<String>(Status::CODE_500, "text/plain").description = "Internal Server Error: An unexpected error occurred.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("POST", "api/v1/video/thumbnail", UploadThumbnail)
            {
                ENDPOINT_ASYNC_INIT(UploadThumbnail);

                Action act() override
                {
                    const auto& body = request->readBodyToString();

                    OATPP_ASSERT_HTTP(!body->empty(), Status::CODE_400, "No content in request body");
                    OATPP_ASSERT_HTTP(request->getHeader("Content-Type") == "image/jpeg", Status::CODE_415, "Invalid file type, expected image/jpeg");

                    m_uuid = request->getQueryParameter("uuid");
                    OATPP_ASSERT_HTTP(m_uuid && !m_uuid->empty(), Status::CODE_400, "UUID is empty");

                    m_userTokenPair = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                    oatpp::String thumbnailPath = VIDEO_FOLDER;
                    thumbnailPath = thumbnailPath + "/" + m_uuid + "/thumbnail.jpg";

                    std::filesystem::create_directory((VIDEO_FOLDER + m_uuid)->c_str());

                    body.saveToFile(thumbnailPath->c_str());

                    return _return(controller->createResponse(Status::CODE_200, "Thumbnail uploaded successfully"));
                }

                bool isBearerAllowedToChange()
                {
                    constexpr char* InfoJsonFileName = "info.json";
                    const std::string & VideoFolderPath = std::string(VIDEO_FOLDER) + "/" + m_uuid;
                    const std::string& InfoJsonFilePath = VideoFolderPath + "/" + InfoJsonFileName;

                    OATPP_ASSERT_HTTP(std::filesystem::exists(VideoFolderPath), Status::CODE_404, "Video not found");
                    OATPP_ASSERT_HTTP(std::filesystem::exists(InfoJsonFilePath), Status::CODE_404, "Info not found");

                    auto JsonFile = oatpp::String::loadFromFile(InfoJsonFilePath.c_str());

                    auto dto = controller->m_objectMapper->readFromString<Object<dto::InfoJsonDTO>>(JsonFile);



                    if (m_userTokenPair->id != dto->userId)
                    {
                        if (dto->channelId != getUserChannelId())
                            return checkForAdminPrivilage();
                        else
                            return true;
                    }
                    else
                        return true;

                }

                UInt64 getUserChannelId()
                {
                    auto dbResult = controller->m_database->getChannelIdByUserId(m_userTokenPair->id);

                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal Server Error");

                    auto resultList = dbResult->fetch<List<List<UInt64>>>();

                    OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal Server Error");

                    if (resultList->empty())
                        return UInt64();
                    else
                        return resultList[0][0];
                }

                bool checkForAdminPrivilage()
                {
                    auto dbResult = controller->m_database->getUserDatabaseEntryById(m_userTokenPair->id);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal Server Error");

                    auto resultList = dbResult->fetch<List<Object<dto::UserDatabaseEntry>>>();
                    OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal Server Error");

                    return resultList[0]->isAdmin;
                }
            private:
                Object<dto::UserTokenPairDTO> m_userTokenPair;
                String                        m_uuid;
            };

            ENDPOINT_INFO(getRandomVideos)
            {
                info->summary = "Get random video list";
                info->description = "Retrieve a list of random video UUIDs. You can specify the number of videos to be returned.";

                info->queryParams.add<String>("amount").description = "The number of random video UUIDs to retrieve. If not provided, all videos are returned.";

                info->addResponse<Object<dto::VideoListDto>>(Status::CODE_200, "application/json")
                    .description = "Successfully retrieved a list of random video UUIDs.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Invalid 'amount' query parameter or request format.";
                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An error occurred while fetching video list.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/list/random", getRandomVideos)
            {
                ENDPOINT_ASYNC_INIT(getRandomVideos);

                Action act() override
                {
                    String AmountString = request->getQueryParameter("amount");

                    const UInt64 Amount = AmountString ? oatpp::utils::Conversion::strToUInt64(AmountString->c_str()) : UINT64_MAX;

                    auto dto = dto::VideoListDto::createShared();

                    dto->uuids = controller->videoIndexer->getFullList();

                    randomizeList(dto->uuids);

                    while (dto->uuids->size() > Amount)
                        dto->uuids->pop_back();

                    dto->size = dto->uuids->size();

                    return _return(controller->createDtoResponse(Status::CODE_200, dto));
                }

                void randomizeList(List<String>& list)
                {
                    std::vector<std::string> vec(list->begin(), list->end());

                    std::random_device rd;
                    std::mt19937 g(rd());

                    std::shuffle(vec.begin(), vec.end(), g);
                    list->assign(vec.begin(), vec.end());
                }
            };

            ENDPOINT_INFO(getVideosOfChannel)
            {
                info->summary = "Get videos of a specific channel";
                info->description = "Retrieve a list of video UUIDs for a given channel, with optional sorting based on criteria such as oldest, likes, dislikes, or views.";

                info->queryParams.add<String>("channelId").description = "The ID of the channel for which to retrieve video UUIDs.";
                info->queryParams.add<String>("amount").description = "The number of videos to return. If not provided, all videos for the channel are returned.";
                info->queryParams.add<String>("sort").description = "The sorting criterion for the videos. Options include 'oldest', 'likes', 'dislikes', 'views', or 'newest' (default).";

                info->addResponse<Object<dto::VideoListDto>>(Status::CODE_200, "application/json")
                    .description = "Successfully retrieved the list of video UUIDs for the specified channel, sorted as requested.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Missing or invalid 'channelId', 'amount', or 'sort' query parameters.";
                info->addResponse<String>(Status::CODE_404, "text/plain")
                    .description = "Not Found: No videos found for the specified channel ID.";
                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An error occurred while fetching or sorting the video list.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/list/channel", getVideosOfChannel)
            {
                ENDPOINT_ASYNC_INIT(getVideosOfChannel);

                Action act() override
                {
                    String ChannelIdString = request->getQueryParameter("channelId");
                    String AmountString    = request->getQueryParameter("amount");
                    String SortByStr       = request->getQueryParameter("sort");

                    OATPP_ASSERT_HTTP(ChannelIdString, Status::CODE_400, "Channel id required");
                    
                    const UInt64 ChannelId = oatpp::utils::Conversion::strToUInt64(ChannelIdString->c_str());
                    const UInt64 Amount = AmountString ? oatpp::utils::Conversion::strToUInt64(AmountString->c_str()) : UINT64_MAX;

                    auto dto = dto::VideoListDto::createShared();

                    dto->uuids = controller->videoIndexer->getChannelList(ChannelId);

                    if (SortByStr == "oldest")
                        controller->videoIndexer->sortBy(VideoIndexer::UUIDSortDate, dto->uuids, VideoIndexer::SortStyleDESC);
                    else if (SortByStr == "likes")
                        controller->videoIndexer->sortBy(VideoIndexer::UUIDSortLikes, dto->uuids, VideoIndexer::SortStyleASC);
                    else if (SortByStr == "dislikes")
                        controller->videoIndexer->sortBy(VideoIndexer::UUIDSortDislikes, dto->uuids, VideoIndexer::SortStyleASC);
                    else if (SortByStr == "views")
                        controller->videoIndexer->sortBy(VideoIndexer::UUIDSortUniqueViews, dto->uuids, VideoIndexer::SortStyleASC);
                    else // Sort by newest date
                        controller->videoIndexer->sortBy(VideoIndexer::UUIDSortDate, dto->uuids, VideoIndexer::SortStyleASC);

                    while (dto->uuids->size() > Amount)
                        dto->uuids->pop_back();

                    dto->size = dto->uuids->size();

                    return _return(controller->createDtoResponse(Status::CODE_200, dto));
                }
            };

            ENDPOINT_INFO(getVideosOfCategory)
            {
                info->summary = "Get videos of a specific category";
                info->description = "Retrieve a list of video UUIDs for a given category, with the option to limit the number of videos returned.";

                info->queryParams.add<String>("category").description = "The category for which to retrieve video UUIDs.";
                info->queryParams.add<String>("amount").description = "The number of videos to return. If not provided, all videos in the category are returned.";

                info->addResponse<Object<dto::VideoListDto>>(Status::CODE_200, "application/json")
                    .description = "Successfully retrieved the list of video UUIDs for the specified category.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Missing or invalid 'category' or 'amount' query parameters.";
                info->addResponse<String>(Status::CODE_404, "text/plain")
                    .description = "Not Found: No videos found for the specified category.";
                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An error occurred while fetching the video list.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/list/category", getVideosOfCategory)
            {
                ENDPOINT_ASYNC_INIT(getVideosOfCategory);

                Action act() override
                {
                    String CategoryStr = request->getQueryParameter("category");
                    String AmountString = request->getQueryParameter("amount");

                    OATPP_ASSERT_HTTP(CategoryStr, Status::CODE_400, "Category required");

                    const UInt64 Amount = AmountString ? oatpp::utils::Conversion::strToUInt64(AmountString->c_str()) : UINT64_MAX;

                    auto dto = dto::VideoListDto::createShared();

                    dto->uuids = controller->videoIndexer->getCategoryList(CategoryStr);

                    while (dto->uuids->size() > Amount)
                        dto->uuids->pop_back();

                    dto->size = dto->uuids->size();

                    return _return(controller->createDtoResponse(Status::CODE_200, dto));
                }
            };

            ENDPOINT_INFO(getSubscriberFeed)
            {
                info->summary = "Get the subscriber feed for the authenticated user";
                info->description = "Retrieve a list of video UUIDs from channels the authenticated user is subscribed to, with the option to limit the number of videos returned.";

                info->queryParams.add<String>("amount").description = "The number of videos to return. If not provided, all videos from subscribed channels are returned.";

                info->addResponse<Object<dto::VideoListDto>>(Status::CODE_200, "application/json")
                    .description = "Successfully retrieved the list of video UUIDs from the user's subscribed channels.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Missing or invalid 'amount' query parameter.";
                info->addResponse<String>(Status::CODE_404, "text/plain")
                    .description = "Not Found: No videos found for the user's subscribed channels.";
                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An error occurred while fetching the subscriber feed.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/list/subscriptions", getSubscriberFeed)
            {
                ENDPOINT_ASYNC_INIT(getSubscriberFeed);

                Action act() override
                {
                    const String& AmountString = request->getQueryParameter("amount");

                    const UInt64 Amount = AmountString ? oatpp::utils::Conversion::strToUInt64(AmountString->c_str()) : UINT64_MAX;

                    auto dto = dto::VideoListDto::createShared();

                    auto subscribedChannels = getSubscribedChannels(controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION))->id);

                    for (auto& channel : *subscribedChannels)
                        for (auto& uuid : *controller->videoIndexer->getChannelList(channel))
                            dto->uuids->push_back(uuid);

                    controller->videoIndexer->sortBy(VideoIndexer::UUIDSortDate, dto->uuids);

                    while (dto->uuids->size() > Amount)
                        dto->uuids->pop_back();

                    dto->size = dto->uuids->size();

                    return _return(controller->createDtoResponse(Status::CODE_200, dto));
                }

                List<UInt64> getSubscribedChannels(const UInt64 userId)
                {
                    auto dbResult = controller->m_database->getSubscribedChannelsForUser(userId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal Server Error");

                    return dbResult->fetch<List<UInt64>>();
                }
            };

            ENDPOINT_INFO(searchVideo)
            {
                info->summary = "Search for videos based on a query string";
                info->description = "This endpoint allows searching for videos by a specified query string. It returns a list of video UUIDs matching the search query.";

                info->queryParams.add<String>("search").description = "The search query to match video titles or metadata.";

                info->addResponse<Object<dto::VideoListDto>>(Status::CODE_200, "application/json")
                    .description = "Successfully retrieved a list of video UUIDs matching the search query.";

                info->addResponse<String>(Status::CODE_400, "text/plain")
                    .description = "Bad Request: Missing or invalid 'search' query parameter.";
                info->addResponse<String>(Status::CODE_404, "text/plain")
                    .description = "Not Found: No videos found matching the search query.";
                info->addResponse<String>(Status::CODE_500, "text/plain")
                    .description = "Internal Server Error: An error occurred while processing the search request.";

                info->addTag("Video");
            }
            ENDPOINT_ASYNC("GET", "api/v1/video/search", searchVideo)
            {
                ENDPOINT_ASYNC_INIT(searchVideo);

                Action act() override
                {
                    const String SearchStr = request->getQueryParameter("search");

                    auto result = controller->videoIndexer->searchFor(SearchStr, 15);

                    return _return(controller->createDtoResponse(Status::CODE_200, result));
                }
            };
           
#include OATPP_CODEGEN_END(ApiController)
        private:
            OATPP_COMPONENT(std::shared_ptr<VideoIndexer>                        , videoIndexer);
            OATPP_COMPONENT(std::shared_ptr<microTube::component::DatabaseClient>, m_database);
            OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>  , m_objectMapper);
        };

    } // namespace apicontroller
} // namespace microTube
