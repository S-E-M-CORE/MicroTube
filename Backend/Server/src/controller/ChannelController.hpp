#ifndef CHANNEL_CONTROLLER_HPP
#define CHANNEL_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"

#include <fstream>
#include <sstream>

namespace microTube {
    namespace apicontroller {

            class ChannelController : public oatpp::web::server::api::ApiController
            {
            public:
                typedef ChannelController __ControllerType;
            public:
                ChannelController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                    : oatpp::web::server::api::ApiController(objectMapper)
                {
                }

            public:
                static std::shared_ptr<ChannelController> createShared(
                    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
                )
                {
                    return std::make_shared<ChannelController>(objectMapper);
                }


#include OATPP_CODEGEN_BEGIN(ApiController)
                ENDPOINT_INFO(GetChannelProfilePicture)
                {
                    info->summary = "Get a channel's profile picture";
                    info->description = "This endpoint retrieves the profile picture for a channel. The `channelId` can be provided as a query parameter. If no `channelId` is given, the channel of the authenticated user is used.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->headers.add<String>("channelId").description = "The ID of the channel whose profile picture is being requested. If not provided, the requesting user's channel is used.";

                    info->addResponse<String>(Status::CODE_200, "image/jpeg")
                        .description = "Successfully retrieved the channel's profile picture.";
                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Profile picture not found: The profile picture for the specified channel ID does not exist.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An error occurred while retrieving the profile picture.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("GET", "api/v1/channel/profile/picture", GetChannelProfilePicture)
                {
                    ENDPOINT_ASYNC_INIT(GetChannelProfilePicture); 

                    Action act() override
                    {
                        oatpp::String channelId = request->getQueryParameter("channelId");
                        if (channelId->empty())
                            channelId = getRequesterChannelIDString();

                        oatpp::String profilePicturePath = CHANNEL_PICTURE_FOLDER + channelId + "/profilepicture.jpg";

                        OATPP_ASSERT_HTTP(std::filesystem::exists(CHANNEL_PICTURE_FOLDER)     , Status::CODE_500, "Internal Server Error");
                        OATPP_ASSERT_HTTP(std::filesystem::exists(profilePicturePath->c_str()), Status::CODE_404, "Profile picture not found");

                        auto fileData = oatpp::String::loadFromFile(profilePicturePath->c_str());
                        return _return(controller->createResponse(Status::CODE_200, fileData));
                    }

                    String getRequesterChannelIDString()
                    {
                        return oatpp::utils::Conversion::uint64ToStr(getRequesterChannelID());
                    }

                    UInt64 getRequesterChannelID()
                    {
                        const auto& requesterId = this->getRequesterId();

                        auto dbResult = controller->m_database->getChannelIdByUserId(requesterId);

                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto dbResultList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "User has no channel");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        return dbResultList[0][0];
                    }

                    UInt64 getRequesterId()
                    {
                        return controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION))->id;
                    }
                };

                ENDPOINT_INFO(UploadChannelProfilePicture)
                {
                    info->summary = "Upload a channel's profile picture";
                    info->description = "This endpoint allows the upload of a profile picture for a channel. The request must include a JPEG file in the body.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully uploaded the channel's profile picture.";
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: The request body is empty.";
                    info->addResponse<String>(Status::CODE_415, "text/plain")
                        .description = "Unsupported Media Type: Invalid file type, expected image/jpeg.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while uploading the profile picture.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("POST", "api/v1/channel/profile/picture", UploadChannelProfilePicture)
                {
                    ENDPOINT_ASYNC_INIT(UploadChannelProfilePicture);

                    Action act() override
                    {
                        return request->readBodyToStringAsync().callbackTo(&UploadChannelProfilePicture::OnBodyRead);
                    }

                    Action OnBodyRead(const String& bodyStr)
                    {
                        OATPP_ASSERT_HTTP(!bodyStr->empty(), Status::CODE_400, "No content in request body");
                        OATPP_ASSERT_HTTP(request->getHeader("Content-Type") == "image/jpeg", Status::CODE_415, "Invalid file type, expected image/jpeg");

                        const auto& IdTokenPair   = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));
                        const auto& UserChannelId = getUserChannelId(IdTokenPair->id);

                        const oatpp::String& profileFolderPath = CHANNEL_PICTURE_FOLDER + oatpp::utils::Conversion::uint64ToStr(UserChannelId);
                        const oatpp::String& profilePicturePath = profileFolderPath + "/profilepicture.jpg";

                        std::filesystem::create_directory(CHANNEL_PICTURE_FOLDER);
                        std::filesystem::create_directory(profileFolderPath->c_str());

                        bodyStr.saveToFile(profilePicturePath->c_str());

                        return _return(controller->createResponse(Status::CODE_200, "Profile picture uploaded successfully"));
                    }

                    UInt64 getUserChannelId(const UInt64 id)
                    {
                        auto dbResult = controller->m_database->getChannelIdByUserId(id);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto dbResultList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "User has no channel");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        return dbResultList[0][0];
                    }
                };
                
                ENDPOINT_INFO(GetChannelProfileBanner)
                {
                    info->summary = "Get a channel's profile banner";
                    info->description = "This endpoint allows retrieving the profile banner of a channel. If no `channelId` is provided as a query parameter, the profile banner of the requester’s channel is returned.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->headers.add<String>("channelId").description = "The ID of the channel whose profile banner is being requested. If not provided, the requesting user's channel banner is returned.";

                    info->addResponse<oatpp::String>(Status::CODE_200, "image/jpeg")
                        .description = "Successfully retrieved the channel's profile banner.";
                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Not Found: The profile banner could not be found.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while retrieving the profile banner.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("GET", "api/v1/channel/profile/banner", GetChannelProfileBanner)
                {
                    ENDPOINT_ASYNC_INIT(GetChannelProfileBanner);

                    Action act() override
                    {
                        oatpp::String channelId = request->getQueryParameter("channelId");
                        if (channelId->empty())
                            channelId = getRequesterChannelIDString();

                        oatpp::String profilePicturePath = CHANNEL_PICTURE_FOLDER + channelId + "/profilebanner.jpg";

                        OATPP_ASSERT_HTTP(std::filesystem::exists(CHANNEL_PICTURE_FOLDER), Status::CODE_500, "Internal Server Error");
                        OATPP_ASSERT_HTTP(std::filesystem::exists(profilePicturePath->c_str()), Status::CODE_404, "Profile picture not found");

                        auto fileData = oatpp::String::loadFromFile(profilePicturePath->c_str());
                        return _return(controller->createResponse(Status::CODE_200, fileData));
                    }

                    String getRequesterChannelIDString()
                    {
                        return oatpp::utils::Conversion::uint64ToStr(getRequesterChannelID());
                    }

                    UInt64 getRequesterChannelID()
                    {
                        const auto& requesterId = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION))->id;

                        auto dbResult = controller->m_database->getChannelIdByUserId(requesterId);

                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto dbResultList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "User has no channel");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        return dbResultList[0][0];
                    }

                };

                ENDPOINT_INFO(UploadChannelProfileBanner)
                {
                    info->summary = "Upload a channel's profile banner";
                    info->description = "This endpoint allows uploading a profile banner for a channel. The request should include the banner image as the body content. The file type must be 'image/jpeg'.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->addConsumes<String>("image/jpeg").description = "JPEG image file for the channel profile banner.";

                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully uploaded the channel profile banner.";
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: The request body is empty or invalid.";
                    info->addResponse<String>(Status::CODE_415, "text/plain")
                        .description = "Unsupported Media Type: The file is not a JPEG image.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while uploading the profile banner.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("POST", "api/v1/channel/profile/banner", UploadChannelProfileBanner)
                {
                    ENDPOINT_ASYNC_INIT(UploadChannelProfileBanner);

                    Action act() override
                    {
                        return request->readBodyToStringAsync().callbackTo(&UploadChannelProfileBanner::OnBodyRead);
                    }

                    Action OnBodyRead(const String & bodyStr)
                    {
                        OATPP_ASSERT_HTTP(!bodyStr->empty(), Status::CODE_400, "No content in request body");
                        OATPP_ASSERT_HTTP(request->getHeader("Content-Type") == "image/jpeg", Status::CODE_415, "Invalid file type, expected image/jpeg");

                        const auto& IdTokenPair = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));
                        const auto& UserChannelId = getUserChannelId(IdTokenPair->id);

                        const oatpp::String& profileFolderPath = CHANNEL_PICTURE_FOLDER + oatpp::utils::Conversion::uint64ToStr(UserChannelId);
                        const oatpp::String& profilePicturePath = profileFolderPath + "/profilebanner.jpg";

                        std::filesystem::create_directory(CHANNEL_PICTURE_FOLDER);
                        std::filesystem::create_directory(profileFolderPath->c_str());

                        bodyStr.saveToFile(profilePicturePath->c_str());

                        return _return(controller->createResponse(Status::CODE_200, "Profile picture uploaded successfully"));
                    }

                    UInt64 getUserChannelId(const UInt64 id)
                    {
                        auto dbResult = controller->m_database->getChannelIdByUserId(id);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());


                        auto dbResultList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "User has no channel");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        return dbResultList[0][0];
                    }
                };

                ENDPOINT_INFO(getSubscriberCount)
                {
                    info->summary = "Get the number of subscribers for a channel";
                    info->description = "This endpoint allows retrieving the number of subscribers for a specific channel. The `channelId` is required as a query parameter.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->headers.add<String>("channelId").description = "The ID of the channel whose subscriber count is being requested.";

                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully retrieved the subscriber count.";
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: The specified channel does not exist or is invalid.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while retrieving the subscriber count.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("GET", "api/v1/channel/subscriber/count", getSubscriberCount)
                {
                    ENDPOINT_ASYNC_INIT(getSubscriberCount);

                    Action act() override
                    {
                        auto channelIdStr = request->getQueryParameter("channelId");

                        OATPP_ASSERT_HTTP(channelIdStr, Status::CODE_400, "Channel id required");

                        const UInt64& requestId = oatpp::utils::Conversion::strToUInt64(channelIdStr->c_str());

                        AssertChannelExists(requestId);

                        auto dbResult = controller->m_database->getSubscriberCountForChannel(requestId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal server error");

                        auto resultList = dbResult->fetch<List<List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!resultList->empty(), Status::CODE_400, "Channel not found");
                        OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal server error");

                        const UInt64 LikeCount = resultList[0][0];

                        return _return(controller->createResponse(Status::CODE_200, oatpp::utils::Conversion::uint64ToStr(LikeCount)));
                    }

                    void AssertChannelExists(const UInt64& channelId)
                    {
                        auto dbResult = controller->m_database->checkChannelExists(channelId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal server error");

                        auto resultList = dbResult->fetch<List<List<oatpp::Int8>>>();

                        OATPP_ASSERT_HTTP(!resultList->empty(), Status::CODE_400, "Channel not found");
                        OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal server error");

                        OATPP_ASSERT_HTTP(static_cast<bool>(resultList[0][0].getValue(0)), Status::CODE_404, "Channel not found");
                    }

                };

                ENDPOINT_INFO(getChannelDescription)
                {
                    info->summary = "Get the description of a channel";
                    info->description = "This endpoint allows retrieving the description of a specific channel. The `channelId` is required as a query parameter.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";
                    info->headers.add<String>("channelId").description = "The ID of the channel whose description is being requested.";

                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully retrieved the channel description.";
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: The specified channel does not exist or is invalid.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while retrieving the channel description.";

                    info->addTag("Channel");
                }
                ENDPOINT_ASYNC("GET", "api/v1/channel/description", getChannelDescription)
                {
                    ENDPOINT_ASYNC_INIT(getChannelDescription);

                    Action act() override
                    {
                        auto channelIdStr = request->getQueryParameter("channelId");

                        OATPP_ASSERT_HTTP(channelIdStr, Status::CODE_400, "Channel id required");

                        const UInt64& requestId = oatpp::utils::Conversion::strToUInt64(channelIdStr->c_str());

                        AssertChannelExists(requestId);

                        auto dbResult = controller->m_database->getChannelDescription(requestId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal server error");

                        auto resultList = dbResult->fetch<List<List<String>>>();

                        OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal server error");

                        const String Description = resultList[0][0];

                        return _return(controller->createResponse(Status::CODE_200, Description));
                    }

                    void AssertChannelExists(const UInt64 & channelId)
                    {
                        auto dbResult = controller->m_database->checkChannelExists(channelId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal server error");

                        auto resultList = dbResult->fetch<List<List<oatpp::Int8>>>();

                        OATPP_ASSERT_HTTP(!resultList->empty(), Status::CODE_400, "Channel not found");
                        OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal server error");

                        OATPP_ASSERT_HTTP(static_cast<bool>(resultList[0][0].getValue(0)), Status::CODE_404, "Channel not found");
                    }

                };


#include OATPP_CODEGEN_END(ApiController)

            private:
                OATPP_COMPONENT(std::shared_ptr<ObjectMapper>                        , m_objectMapper);
                OATPP_COMPONENT(std::shared_ptr<microTube::component::DatabaseClient>, m_database);
            };


    } // namespace apicontroller
} // namespace microTube

#endif // CHANNEL_CONTROLLER_HPP