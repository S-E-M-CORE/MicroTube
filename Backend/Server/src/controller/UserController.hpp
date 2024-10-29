#include "dto/StatusDTO.hpp"
#include "dto/UserDTO.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"

namespace microTube {
    namespace apicontroller {
        namespace user_endpoint {

#include OATPP_CODEGEN_BEGIN(ApiController)
            class UserController : public oatpp::web::server::api::ApiController
            {
            public:
                UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                    : oatpp::web::server::api::ApiController(objectMapper)
                {
                }

            public:
                static std::shared_ptr<UserController> createShared(
                    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
                )
                {
                    return std::make_shared<UserController>(objectMapper);
                }

                ENDPOINT_ASYNC("GET", "/api/v1/user/register", RegisterUser) {
                    ENDPOINT_ASYNC_INIT(RegisterUser);

                    Action act() override {

                        return _return(controller->createResponse(Status::CODE_501));
                    }
                };

                ENDPOINT_INFO(RegisterUser) {
                    info->summary = "Registers a user";
                    info->description = "Registers a user";
                    info->addTag("User");
                    info->addTag("Unimplemented");
                }

                ENDPOINT_ASYNC("GET", "/api/v1/user/login", LoginUser) {
                    ENDPOINT_ASYNC_INIT(LoginUser);

                    Action act() override {

                        return _return(controller->createResponse(Status::CODE_501));
                    }
                };

                ENDPOINT_INFO(LoginUser) {
                    info->summary = "Logs a user in";
                    info->addTag("User");
                    info->addTag("Unimplemented");
                }

                ENDPOINT_ASYNC("GET", "/api/v1/user/", GetInfo) {
                    ENDPOINT_ASYNC_INIT(GetInfo);

                    Action act() override {

                        return _return(controller->createResponse(Status::CODE_501));
                    }
                };

                ENDPOINT_INFO(GetInfo) {
                    info->summary = "Retrieves the users info.";
                    info->addTag("User");
                    info->addTag("Unimplemented");
                }
            };

#include OATPP_CODEGEN_END(ApiController)

        } // namespace static_endpoint
    } // namespace apicontroller
} // namespace microTube
