#ifndef AppComponent_hpp
#define AppComponent_hpp

// Oatpp headers
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/macro/component.hpp"
#include "videos/VideoIndexer.hpp"

// App specific headers
#include "swagger-ui/SwaggerComponent.hpp"
#include "sql/DatabaseComponent.hpp"

namespace microTube
{
    namespace component
    {
        class AppComponent
        {
        public:
            microtube::component::DatabaseComponent databaseComponent;
            SwaggerComponent                        swaggerComponent;

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([]
                {
                return oatpp::network::tcp::server::ConnectionProvider::createShared({ "0.0.0.0", 8000, oatpp::network::Address::IP_4 });
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([]
                {
                return oatpp::web::server::HttpRouter::createShared();
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
                OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
                return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router);
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
                ([] {
                    return std::make_shared<oatpp::json::ObjectMapper>();
                }());

            OATPP_CREATE_COMPONENT(std::shared_ptr<microTube::video::VideoIndexer>, videoIndexer)
                ([]{
                    return std::make_shared<microTube::video::VideoIndexer>(VIDEO_FOLDER);
                }());


        };

    } //namespace component
} // namespace microTube

#endif /* AppComponent_hpp */
