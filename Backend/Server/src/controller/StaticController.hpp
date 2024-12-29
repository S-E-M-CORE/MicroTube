#ifndef STATIC_CONTROLLER_HPP
#define STATIC_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"
#include <fstream>
#include <sstream>

namespace microTube {
    namespace apicontroller {

#include OATPP_CODEGEN_BEGIN(ApiController)
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

                ENDPOINT_INFO(files)
                {
                    info->summary = "Serve static web files";
                    info->description = "This endpoint serves static files from the server's web folder. If no file path is provided, the server returns the `index.html` file. The file path can be specified as part of the URL. Query parameters are ignored.";

                    info->addResponse<String>(Status::CODE_200, "text/html")
                        .description = "Successfully served the requested static file.";
                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Not Found: The requested file could not be found.";

                    info->addTag("Web");
                }
                ENDPOINT_ASYNC("GET", "web/*", files)
                {
                    ENDPOINT_ASYNC_INIT(files)

                        Action act() override
                    {
                        // Ignore query parameters if present
                        oatpp::String       pathTail = request->getPathTail();
                        const std::size_t   queryPos = pathTail->find("?");

                        if (queryPos != std::string::npos)
                        {
                            pathTail = pathTail->substr(0, queryPos);
                        }

                        std::string filePath(WEB_FOLDER);
                        filePath.append("/");

                        if (pathTail->empty())  filePath.append("index.html");
                        else                    filePath.append(*pathTail);

                        std::ifstream file(filePath, std::ios::binary);
                        if (file.good())
                        {
                            std::ostringstream content;
                            content << file.rdbuf();
                            file.close();

                            return _return(controller->createResponse(Status::CODE_200, content.str()));
                        }
                        else
                        {
                            const std::string& verboseMessage = std::string("File at \"") + filePath + "\" could not be found";
                            return _return(controller->createResponse(Status::CODE_404, verboseMessage));
                        }
                    }
                };

                ENDPOINT_INFO(root)
                {
                    info->summary = "Redirect to web folder";
                    info->description = "This endpoint redirects any request made to the root URL (`/`) to the `/web/` directory.";

                    info->addResponse<String>(Status::CODE_302, "text/plain")
                        .description = "Redirects the client to the `/web/` directory.";

                    info->addTag("Web");
                }
                ENDPOINT_ASYNC("GET", "/", root)
                {
                    ENDPOINT_ASYNC_INIT(root);

                    Action act() override
                    {
                        auto response = this->controller->createResponse(Status::CODE_302, "Redirect");
                        response->putHeader("Location", "/web/");

                        return _return(response);
                    }
                };
            };

#include OATPP_CODEGEN_END(ApiController)

    } // namespace apicontroller
} // namespace microTube


#endif // STATIC_CONTROLLER_HPP