#include "dto/StatusDTO.hpp"
#include "dto/VideoInfoDTO.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"

namespace microTube {
    namespace apicontroller {
        namespace video_endpoint {

            #include OATPP_CODEGEN_BEGIN(ApiController)
            class VideoController : public oatpp::web::server::api::ApiController
            {
            public:
                VideoController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                    : oatpp::web::server::api::ApiController(objectMapper)
                {
                }

            public:
                static std::shared_ptr<VideoController> createShared(
                    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
                )
                {
                    return std::make_shared<VideoController>(objectMapper);
                }

                ENDPOINT_INFO(GetVideoStream) {
                    info->summary = "Stream video file";
                    info->description = "Streams a video file to the client using asynchronous processing. "
                        "The video file is identified by the filename provided in the path.";
                    info->pathParams["filename"].name        = "filename";
                    info->pathParams["filename"].description = "The name of the video file to stream.";
                    info->pathParams["filename"].required    = true;
                    info->addResponse<oatpp::String>(Status::CODE_200, "video/mp4", "Video data as a stream");
                    info->addResponse<oatpp::String>(Status::CODE_404, "text/plain", "Video not found");
                    info->addResponse<oatpp::String>(Status::CODE_500, "text/plain", "Internal server error");
                    info->addResponse<oatpp::String>(Status::CODE_501, "text/plain", "Endpoint not yet implemented");
                    info->addTag("Video");
                    info->addTag("Unimplemented");
                }

                ENDPOINT_ASYNC("GET", "/api/v1/video/stream/{filename}", GetVideoStream) {
                    ENDPOINT_ASYNC_INIT(GetVideoStream);

                    Action act() override {
                        auto filename = request->getPathVariable("filename");
                        oatpp::String filePath = "path/to/videos/" + filename;
                        auto fileStream = std::make_shared<std::ifstream>(filePath->c_str(), std::ios::binary);

                        if (!fileStream->is_open()) {
                            return _return(controller->createResponse(Status::CODE_404, "Video not found"));
                        }

                        auto response = controller->createResponse(Status::CODE_501);
                        response->putHeader("Content-Type", "text/plain"); // Adjust content type according to the video format
                        return _return(response);
                    }
                };

                ENDPOINT_ASYNC("GET", "/api/v1/video/info/{filename}", GetVideoInfo) {
                    ENDPOINT_ASYNC_INIT(GetVideoInfo);

                    Action act() override {
                        auto filename = request->getPathVariable("filename");
                        oatpp::String filePath = "path/to/videos/" + filename;
                        auto fileStream = std::make_shared<std::ifstream>(filePath->c_str(), std::ios::binary);

                        if (!fileStream->is_open())
                        {
                            return _return(controller->createResponse(Status::CODE_404, "Video not found"));
                        }

                        auto response = controller->createResponse(Status::CODE_501);
                        response->putHeader("Content-Type", "text/plain"); // Adjust content type according to the video format
                        return _return(response);
                    }
                };

                ENDPOINT_INFO(GetVideoInfo) {
                    info->summary = "Get video info";
                    info->description = "Retrieves information related to a video, like its title, description, upload date, etc.";
                    info->pathParams["filename"].name = "filename";
                    info->pathParams["filename"].description = "The name of the video file.";
                    info->pathParams["filename"].required = true;
                    info->addResponse<Object<microTube::dto::VideoInfoDTO>>(Status::CODE_200, "application/json", "Video data as a stream");
                    info->addResponse<oatpp::String>(Status::CODE_404, "text/plain", "Video not found");
                    info->addResponse<oatpp::String>(Status::CODE_500, "text/plain", "Internal server error");
                    info->addResponse<oatpp::String>(Status::CODE_501, "text/plain", "Endpoint not yet implemented");
                    info->addTag("Video");
                    info->addTag("Unimplemented");
                }
            };

#include OATPP_CODEGEN_END(ApiController)

        } // namespace static_endpoint
    } // namespace apicontroller
} // namespace microTube
