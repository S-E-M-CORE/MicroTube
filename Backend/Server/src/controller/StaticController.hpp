#include "dto/StatusDTO.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"
#include <fstream>
#include <sstream>

namespace microTube {
    namespace apicontroller {
        namespace static_endpoint {

#include OATPP_CODEGEN_BEGIN(ApiController)
            //  ____  _        _   _       ____            _             _ _           
            // / ___|| |_ __ _| |_(_) ___ / ___|___  _ __ | |_ _ __ ___ | | | ___ _ __ 
            // \___ \| __/ _` | __| |/ __| |   / _ \| '_ \| __| '__/ _ \| | |/ _ \ '__|
            //  ___) | || (_| | |_| | (__| |__| (_) | | | | |_| | | (_) | | |  __/ |   
            // |____/ \__\__,_|\__|_|\___|\____\___/|_| |_|\__|_|  \___/|_|_|\___|_|   
            class StaticController : public oatpp::web::server::api::ApiController
            {
            public:
                StaticController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                    : oatpp::web::server::api::ApiController(objectMapper)
                {
                }

            public:
                static std::shared_ptr<StaticController> createShared(
                    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
                )
                {
                    return std::make_shared<StaticController>(objectMapper);
                }

                ENDPOINT("GET", "/web/*", files,
                    REQUEST(std::shared_ptr<IncomingRequest>, request))
                {
                    // Ignore query parameters if present
                    auto pathTail = request->getPathTail();
                    auto queryPos = pathTail->find("?");
                    if (queryPos != std::string::npos) {
                        pathTail = pathTail->substr(0, queryPos);
                    }

                    std::string filePath(WEB_CONTENT_DIRECTORY);
                    filePath.append("/");

                    if (pathTail->empty()) {
                        filePath.append("index.html");
                    }
                    else {
                        filePath.append(*pathTail);
                    }

                    std::ifstream file(filePath, std::ios::binary);
                    if (file.good()) {
                        std::ostringstream content;
                        content << file.rdbuf();
                        file.close();

                        return createResponse(Status::CODE_200, content.str());
                    }
                    else {
                        auto status = microTube::dto::StatusDto::createShared();

                        std::string verboseMessage = "File at \"";
                        verboseMessage.append(filePath.c_str());
                        verboseMessage.append("\" could not be found");

                        status->code = 404;
                        status->message = verboseMessage;
                        status->status = "NOT FOUND";
                        return createDtoResponse(Status::CODE_404, status);
                    }
                }

                ENDPOINT_INFO(files)
                {
                    info->name = "files";
                    info->summary = "Serve static files";
                    info->description = "This endpoint serves static files from the '/web' directory.";
                    info->path = "/web/*";
                    info->method = "GET";
                    info->addTag("Static");
                    info->pathParams["*"].description = "File path relative to the '/web' directory";
                    info->addResponse<String>(Status::CODE_200, "text/html");
                    info->addResponse<Object<microTube::dto::StatusDto>>(Status::CODE_404, "application/json");
                }


                ENDPOINT("GET", "/", root,
                    REQUEST(std::shared_ptr<IncomingRequest>, request))
                {
                    auto response = createResponse(Status::CODE_302, "Redirect");
                    response->putHeader("Location", "/web/");

                    return response;
                }

                ENDPOINT_INFO(root)
                {
                    info->name = "root";
                    info->summary = "Redirect to web directory";
                    info->description = "This endpoint redirects requests to the root URL to the '/web' directory.";
                    info->path = "/";
                    info->addTag("Static");
                    info->method = "GET";
                    info->addResponse<String>(Status::CODE_302, "text/html");
                }

            };

#include OATPP_CODEGEN_END(ApiController)

        } // namespace static_endpoint
    } // namespace apicontroller
} // namespace microTube
