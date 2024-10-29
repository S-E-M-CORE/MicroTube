#include "AppComponent.hpp"

// Controller includes
#include "general/constants.hpp"
#include "AppComponent.hpp"
#include "controller/StaticController.hpp"
#include "controller/VideoController.hpp"
#include "controller/UserController.hpp"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/network/Server.hpp"
#include <iostream>

namespace microTube {
    namespace main {
        void run(void) {
            constexpr const char* const logName = microTube::constants::defaultLogName;

            microTube::component::AppComponent components{};

            OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

            // Endpoint controllers
            auto staticController = microTube::apicontroller::static_endpoint::StaticController::createShared();
            auto userController = microTube::apicontroller::user_endpoint::UserController::createShared();
            auto videoController  = microTube::apicontroller::video_endpoint ::VideoController ::createShared();

            router->addController(staticController);
            router->addController(userController);
            router->addController(videoController);

            if (microTube::constants::useSwaggerUi)
            {
                oatpp::web::server::api::Endpoints docEndpoints{};
                docEndpoints.append(staticController->getEndpoints());
                docEndpoints.append(userController  ->getEndpoints());
                docEndpoints.append(videoController ->getEndpoints());
                router->addController(oatpp::swagger::Controller::createShared(std::move(docEndpoints)));
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

            server.run();
        }
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