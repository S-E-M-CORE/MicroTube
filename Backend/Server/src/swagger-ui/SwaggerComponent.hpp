
#ifndef SwaggerComponent_hpp
#define SwaggerComponent_hpp

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"

namespace microTube
{
    namespace component
    {
        class SwaggerComponent {
        public:

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)([] {

                oatpp::swagger::DocumentInfo::Builder builder;

                builder
                    .setTitle("MicroTube")
                    .setDescription("Swagger documentation for the MicroTube project")
                    .setVersion("1.0")
                    .setContactName("Sascha Meissner")
                    .setContactUrl("https://s-e-m-core.github.io/Website/")

                    .setLicenseName("GNU GENERAL PUBLIC LICENSE Version 3")

                    .addServer("http://localhost:8000", "server on localhost");

                return builder.build();

                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)([] {
                return oatpp::swagger::Resources::loadResources(OATPP_SWAGGER_RES_PATH);
                }());
        };
    } //namespace component
} // namespace microTube

#endif /* SwaggerComponent_hpp */
