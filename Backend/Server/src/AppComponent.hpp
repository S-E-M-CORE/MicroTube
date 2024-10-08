#ifndef AppComponent_hpp
#define AppComponent_hpp

// Oatpp headers
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"

// App specific headers
#include "swagger-ui/SwaggerComponent.hpp"

namespace microTube
{
    namespace component
    {
        class AppComponent
        {
        public:
            SwaggerComponent swaggerComponent;

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
                return oatpp::network::tcp::server::ConnectionProvider::createShared({ "0.0.0.0", 8000, oatpp::network::Address::IP_4 });
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
                return oatpp::web::server::HttpRouter::createShared();
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
                OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
                return oatpp::web::server::HttpConnectionHandler::createShared(router);
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
                return oatpp::parser::json::mapping::ObjectMapper::createShared();
                }());
        };

    } //namespace component
} // namespace microTube

#endif /* AppComponent_hpp */
