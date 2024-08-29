#include "AppComponent.hpp"

// Controller includes
#include "general/constants.hpp"
#include "AppComponent.hpp"
#include "controller/StaticController.hpp"
#include "controller/VideoController.hpp"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/network/Server.hpp"
#include <iostream>

namespace microTube {
    namespace main {
        void run(void) {
            constexpr const char* const logName = microTube::constants::defaultLogName;

            /* Register Components in scope of run() method */
            microTube::component::AppComponent components{};

            /* Get router component */
            OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

            /* Create StaticController and add all of its endpoints to router */
            router->addController(std::make_shared<microTube::apicontroller::static_endpoint::StaticController>());

            router->addController(std::make_shared<microTube::apicontroller::video_endpoint::VideoController>());

            if (microTube::constants::useSwaggerUi)
            {
                /* Swagger UI Endpoint documentation */
                oatpp::web::server::api::Endpoints docEndpoints;

                docEndpoints.append(router->addController(microTube::apicontroller::static_endpoint::StaticController::createShared())->getEndpoints());

                docEndpoints.append(router->addController(microTube::apicontroller::video_endpoint::VideoController::createShared())->getEndpoints());

                router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
            }

            /* Get connection handler component */
            OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

            /* Get connection provider component */
            OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

            /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
            oatpp::network::Server server(connectionProvider, connectionHandler);
            {
                const char* host{static_cast<const char*>(connectionProvider->getProperty("host").getData())};
                const char* port{ static_cast<const char*>(connectionProvider->getProperty("port").getData()) };

                host = host[0] == 48 && host[1] == 46 ? "localhost" : host;

                OATPP_LOGI(logName, "MicroTube server at http://%s:%s/web/", host, port);

                if(microTube::constants::useSwaggerUi)
                    OATPP_LOGI(logName, "Swagger-ui at http://%s:%s/swagger/ui", host, port);
            }
            /* Run server */
            server.run();

        } // run()
    } // namespace main
} // namespace microTube

int main(int argc, const char* argv[])
{
    oatpp::base::Environment::init();

    microTube::main::run();

    std::cout << "\nEnvironment:\n";
    std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
    std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

    oatpp::base::Environment::destroy();

    return 0;
}