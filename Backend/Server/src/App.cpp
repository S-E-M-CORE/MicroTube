#include "AppComponent.hpp"

#include "controller/StaticController.hpp"
#include "controller/VideoController.hpp"
#include "controller/UserController.hpp"
#include "controller/ChannelController.hpp"
#include "oatpp-swagger/AsyncController.hpp"
#include "oatpp/network/Server.hpp"

void run()
{
    const bool                         useSwagger{ true };
    microTube::component::AppComponent components{};

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    // Endpoint controllers
    auto staticController  = microTube::apicontroller::StaticController::createShared();
    auto userController    = microTube::apicontroller::UserController  ::createShared();
    auto videoController   = microTube::apicontroller::VideoController::createShared();
    auto channelController = microTube::apicontroller::ChannelController::createShared();

    router->addController(staticController);
    router->addController(userController);
    router->addController(videoController);
    router->addController(channelController);

    if (useSwagger)
    {
        oatpp::web::server::api::Endpoints docEndpoints{};
        docEndpoints.append(staticController ->getEndpoints());
        docEndpoints.append(userController   ->getEndpoints());
        docEndpoints.append(videoController  ->getEndpoints());
        docEndpoints.append(channelController->getEndpoints());
        router->addController(oatpp::swagger::AsyncController::createShared(std::move(docEndpoints)));
    }

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    const char* host{static_cast<const char*>(connectionProvider->getProperty("host").getData())};
    const char* port{ static_cast<const char*>(connectionProvider->getProperty("port").getData()) };

    host = host[0] == 48 && host[1] == 46 ? "localhost" : host;

    OATPP_LOGi("INIT", std::string("MicroTube server at ") + std::string(host) + ":" + std::string(port) + "/web/");

    if(useSwagger)
        OATPP_LOGi("INIT", std::string("Swagger-ui at ") + host + ":" + port + "/swagger/ui/");

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);


    server.run();
}

int main(int /*argc*/, const char* /*argv[]*/)
{
    oatpp::Environment::init();

    run();

    oatpp::Environment::destroy();
    return 0;
}