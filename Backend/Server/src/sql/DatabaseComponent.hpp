#ifndef CRUD_DATABASECOMPONENT_HPP
#define CRUD_DATABASECOMPONENT_HPP

#include "oatpp/macro/component.hpp"

#include "oatpp-sqlite/Connection.hpp"
#include "oatpp-sqlite/ConnectionProvider.hpp"
#include "oatpp-sqlite/Executor.hpp"

#include "DatabaseClient.hpp"

namespace microtube
{
    namespace component
    {
        class DatabaseComponent
        {
        public:
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, dbConnectionProvider)([] {

                auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(DATABASE_FILE);

                return oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                    10 /* max-connections */,
                    std::chrono::seconds(5) /* connection TTL */);
                }());

            // Create database client
            OATPP_CREATE_COMPONENT(std::shared_ptr<microTube::component::DatabaseClient>, database)([] {

                /* Get database ConnectionProvider component */
                OATPP_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, connectionProvider);

                /* Create database-specific Executor */
                auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionProvider);

                /* Create MyClient database client */
                return std::make_shared<microTube::component::DatabaseClient>(executor);

                }());

        };

    } //namespace component
} // namespace microtube

#endif //CRUD_DATABASECOMPONENT_HPP
