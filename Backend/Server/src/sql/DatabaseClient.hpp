#ifndef DATABASE_CLIENT
#define DATABASE_CLIENT

#include "oatpp/orm/SchemaMigration.hpp"
#include "oatpp/orm/DbClient.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/base/Log.hpp"

#include "../dto/UserDTO.hpp"
#include "../dto/InfoJsonDTO.hpp"
#include "../dto/VideoListDTO.hpp"

namespace dto = microTube::dto;
using Status = oatpp::web::protocol::http::Status;

namespace microTube
{
    namespace component
    {
#include OATPP_CODEGEN_BEGIN(DbClient) //<- Begin Codegen
        class DatabaseClient : public oatpp::orm::DbClient
        {
        public:
            DatabaseClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
                : oatpp::orm::DbClient(executor)
            {
                OATPP_LOGi("Database", "DatabaseClient(oatpp::orm::DbClient) initialized");

                oatpp::orm::SchemaMigration migration(executor);
                migration.addFile(1, DATABASE_MIGRATIONS "/001_init.sql");
                migration.migrate(); // <-- run migrations. This guy will throw on error.

                auto version = executor->getSchemaVersion();
                OATPP_LOGi("Database", "Migration - OK. Version={}.", version);
            }

            QUERY(registerUser,
                "INSERT INTO User (email, password, nickname, firstname, lastname, phonenumber, birthDate, joinDate, "
                "secretQuestion1, secretQuestion2, secretQuestion3, secretAnswer1, secretAnswer2, secretAnswer3, isAdmin, isActive) "
                "VALUES (:user.email, :user.password, :user.nickname, :user.firstname, :user.lastname, :user.phonenumber, :user.birthDate, "
                "DATE('now'), :user.secretQuestion1, :user.secretQuestion2, :user.secretQuestion3, "
                ":user.secretAnswer1, :user.secretAnswer2, :user.secretAnswer3, FALSE, TRUE) "
                "RETURNING id;",
                PARAM(oatpp::Object<microTube::dto::UserRegistrationDTO>, user));

            QUERY(getUserDatabaseEntryById,
                "SELECT id, email, password, nickname, firstname, lastname, phonenumber, birthDate, joinDate, secretQuestion1, secretQuestion2, secretQuestion3, secretAnswer1, secretAnswer2, secretAnswer3, isAdmin, isActive "
                "FROM User "
                "WHERE id = :id;",
                PARAM(oatpp::UInt64, id));

            QUERY(getUserInfoById,
                "SELECT email, nickname, firstname, lastname, phonenumber, birthDate "
                "FROM User "
                "WHERE id = :id;",
                PARAM(oatpp::UInt64, id));

            QUERY(getUserIdByEmail,
                "SELECT id "
                "FROM User "
                "WHERE email = :email;",
                PARAM(oatpp::String, email));

            QUERY(getUserByIdAndPassword,
                "SELECT * "
                "FROM User "
                "WHERE id = :id AND password = :password;",
                PARAM(oatpp::UInt64, id),
                PARAM(oatpp::String, password));

            QUERY(makeUserLikeVideo,
                "INSERT INTO Video_Likes (uuid, user_id) "
                "VALUES (:videoId, :userId) "
                "ON CONFLICT (uuid, user_id) DO NOTHING;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));

            QUERY(getLikeCountForVideo,
                "SELECT COUNT(*) AS likeCount "
                "FROM Video_Likes "
                "WHERE uuid = :videoId;",
                PARAM(oatpp::String, videoId));

            QUERY(getDislikeCountForVideo,
                "SELECT COUNT(*) AS dislikeCount "
                "FROM Video_Disikes "
                "WHERE uuid = :videoId;",
                PARAM(oatpp::String, videoId));

            QUERY(getUniqueViewCountForVideo,
                "SELECT COUNT(*) AS uniqueViewCount "
                "FROM Videos_Unique_Views "
                "WHERE uuid = :videoId;",
                PARAM(oatpp::String, videoId));

            QUERY(makeUserDislikeVideo,
                "INSERT INTO Video_Disikes (uuid, user_id) "
                "VALUES (:videoId, :userId) "
                "ON CONFLICT (uuid, user_id) DO NOTHING;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));

            QUERY(removeLike,
                "DELETE FROM Video_Likes "
                "WHERE uuid = :videoId AND user_id = :userId;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));

            QUERY(removeDislike,
                "DELETE FROM Video_Disikes "
                "WHERE uuid = :videoId AND user_id = :userId;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));

            QUERY(addSubscription,
                "INSERT INTO User_Channel_Subscriptions (memberID, creatorID) "
                "VALUES (:memberId, :creatorId) "
                "ON CONFLICT (memberID, creatorID) DO NOTHING;",
                PARAM(oatpp::UInt64, memberId),
                PARAM(oatpp::UInt64, creatorId));

            QUERY(removeSubscription,
                "DELETE FROM User_Channel_Subscriptions "
                "WHERE memberID = :memberId AND creatorID = :creatorId;",
                PARAM(oatpp::UInt64, memberId),
                PARAM(oatpp::UInt64, creatorId));

            QUERY(getSubscriberCountForChannel,
                "SELECT COUNT(*) AS subscriberCount "
                "FROM User_Channel_Subscriptions "
                "WHERE creatorID = :channelId;",
                PARAM(oatpp::UInt64, channelId));

            QUERY(checkChannelExists,
                "SELECT EXISTS ("
                "  SELECT 1 "
                "  FROM Channel "
                "  WHERE id = :channelId"
                ") AS channelExists;",
                PARAM(oatpp::UInt64, channelId));

            QUERY(getChannelDescription,
                "SELECT description "
                "FROM Channel "
                "WHERE id = :channelId;",
                PARAM(oatpp::UInt64, channelId));

            QUERY(setChannelDescription,
                "UPDATE Channel "
                "SET description = :newDescription "
                "WHERE id = :channelId;",
                PARAM(oatpp::String, newDescription),
                PARAM(oatpp::UInt64, channelId));

            QUERY(hasUserViewedVideo,
                "SELECT EXISTS ("
                "  SELECT 1 "
                "  FROM Videos_Unique_Views "
                "  WHERE uuid = :videoId AND user_id = :userId"
                ") AS hasViewed;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));


            QUERY(addUniqueView,
                "INSERT INTO Videos_Unique_Views (uuid, user_id) "
                "SELECT :videoId, :userId "
                "WHERE :videoId IS NOT NULL AND :userId IS NOT NULL AND :videoId != '' "
                "ON CONFLICT (uuid, user_id) DO NOTHING;",
                PARAM(oatpp::String, videoId),
                PARAM(oatpp::UInt64, userId));


            QUERY(getUniqueViewCount,
                "SELECT COUNT(*) AS viewCount "
                "FROM Videos_Unique_Views "
                "WHERE uuid = :videoId;",
                PARAM(oatpp::String, videoId));

            QUERY(isTokenValid,
                "SELECT token, user_id AS id "
                "FROM User_Tokens "
                "WHERE token = :token "
                "AND validTillDate > DATE('now');",
                PARAM(oatpp::String, token));

            QUERY(insertNewToken,
                "INSERT INTO User_Tokens (token, user_id, createDate, validTillDate) "
                "VALUES (:token, :userId, DATE('now'), DATE('now', '+30 days'));",
                PARAM(oatpp::String, token),
                PARAM(oatpp::UInt64, userId));

            QUERY(extendTokenValidity,
                "UPDATE User_Tokens "
                "SET validTillDate = DATE('now', '+30 days') "
                "WHERE token = :token "
                "AND validTillDate > DATE('now');",
                PARAM(oatpp::String, token));

            QUERY(getUserIdByToken,
                "SELECT user_id "
                "FROM User_Tokens "
                "WHERE token = :token "
                "AND validTillDate > DATE('now');",
                PARAM(oatpp::String, token));

            QUERY(getValidTokens,
                "SELECT token "
                "FROM User_Tokens "
                "WHERE user_id = :userId "
                "AND validTillDate > DATE('now');",
                PARAM(oatpp::UInt64, userId));

            QUERY(updateChannelDescriptionByUserId,
                "UPDATE Channel "
                "SET description = :newDescription "
                "WHERE id = ("
                "  SELECT channelId "
                "  FROM UserChannelJunction "
                "  WHERE user_id = :userId "
                "  LIMIT 1"
                ");",
                PARAM(oatpp::String, newDescription),
                PARAM(oatpp::UInt64, userId));

            QUERY(updateChannelDescriptionById,
                "UPDATE Channel "
                "SET description = :newDescription "
                "WHERE id = :channelId;",
                PARAM(oatpp::String, newDescription),
                PARAM(oatpp::UInt64, channelId));

            QUERY(createNewChannel,
                "INSERT INTO Channel (description) "
                "VALUES (:description) "
                "RETURNING id;",
                PARAM(oatpp::String, description));

            QUERY(createUserChannelJunction,
                "INSERT INTO UserChannelJunction (channelId, user_id) "
                "VALUES (:channelId, :userId) "
                "ON CONFLICT (channelId, user_id) DO NOTHING;",
                PARAM(oatpp::UInt64, channelId),
                PARAM(oatpp::UInt64, userId));

            QUERY(createChannelWithDescription,
                "INSERT INTO Channel (description) "
                "VALUES (:description) "
                "RETURNING id;",
                PARAM(oatpp::String, description));

            QUERY(updateUserInfo,
                "UPDATE User "
                "SET email = :user.email, "
                "    password = :user.password, "
                "    nickname = :user.nickname, "
                "    firstname = :user.firstname, "
                "    lastname = :user.lastname, "
                "    phonenumber = :user.phonenumber, "
                "    birthDate = :user.birthDate, "
                "    secretQuestion1 = :user.secretQuestion1, "
                "    secretQuestion2 = :user.secretQuestion2, "
                "    secretQuestion3 = :user.secretQuestion3, "
                "    secretAnswer1 = :user.secretAnswer1, "
                "    secretAnswer2 = :user.secretAnswer2, "
                "    secretAnswer3 = :user.secretAnswer3, "
                "    isAdmin = :user.isAdmin, "
                "    isActive = :user.isActive "
                "WHERE id = :id;",
                PARAM(oatpp::Object<microTube::dto::UserDatabaseEntry>, user),
                PARAM(oatpp::UInt64, id));
        
            QUERY(getChannelIdByUserId,
                "SELECT channelId "
                "FROM UserChannelJunction "
                "WHERE user_id = :userId "
                "LIMIT 1;",
                PARAM(oatpp::UInt64, userId));

            QUERY(getSubscribedChannelsForUser,
                "SELECT c.id, c.description "
                "FROM User_Channel_Subscriptions ucs "
                "INNER JOIN Channel c ON ucs.creatorID = c.id "
                "WHERE ucs.memberID = :userId;",
                PARAM(oatpp::UInt64, userId));

            oatpp::Object<dto::UserTokenPairDTO> isBearerTokenValid(oatpp::String tokenStr)
            {
                if (!tokenStr)
                    return nullptr;

                const size_t TokenStart = tokenStr->find_last_of(" ");
                const oatpp::String Token = tokenStr->substr(TokenStart);

                auto dbResult = isTokenValid(Token);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                auto dbResultList = dbResult->fetch<oatpp::List<oatpp::Object<dto::UserTokenPairDTO>>>();

                if (dbResultList->empty())
                    return nullptr;

                OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                return dbResultList[0];
            }

            oatpp::Object<dto::UserTokenPairDTO> checkBearerThrowIfInvalid(oatpp::String tokenStr)
            {
                OATPP_ASSERT_HTTP(tokenStr, Status::CODE_400, "Bearer required");

                const size_t TokenStart = tokenStr->find_last_of(" ");
                const oatpp::String Token = tokenStr->substr(TokenStart + 1);

                auto dbResult = isTokenValid(Token);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                auto dbResultList = dbResult->fetch<oatpp::List<oatpp::Object<dto::UserTokenPairDTO>>>();

                OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");
                OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "Unknown token");

                return dbResultList[0];
            }

            bool isUserAdmin(const oatpp::UInt64 userId)
            {
                auto dbResult = getUserDatabaseEntryById(userId);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Internal Server Error");

                auto resultList = dbResult->fetch<oatpp::List<oatpp::Object<dto::UserDatabaseEntry>>>();
                OATPP_ASSERT_HTTP(resultList->size() == 1, Status::CODE_500, "Internal Server Error");

                return resultList[0]->isAdmin;
            }
};

#include OATPP_CODEGEN_END(DbClient) ///< End code-gen section


    } // namespace component
} // namespace microtube

#endif //DATABASE_CLIENT