#include "dto/UserDTO.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"
#include "../sql/DatabaseClient.hpp"

#include <iostream>
#include <string>
#include <random>

#include "filesystem"
#include <cassert>
#include <future>

namespace microTube {
    namespace apicontroller {

            namespace dto = microTube::dto;
#include OATPP_CODEGEN_BEGIN(ApiController)
            class UserController : public oatpp::web::server::api::ApiController
            {
            public:
                typedef UserController __ControllerType;
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

                ENDPOINT_INFO(RegisterUser)
                {
                    info->summary = "Register a new user";
                    info->description = "This endpoint allows registering a new user with essential details like email, password, and personal information.";

                    info->addConsumes<Object<microTube::dto::UserRegistrationDTO>>("application/json", "Information of the new user to be registered");

                    auto example = microTube::dto::UserRegistrationDTO::createShared();
                    example->email = "max.mustermann@muster.com";
                    example->password = "12345";
                    example->nickname = "MaxMusti";
                    example->firstname = "Max";
                    example->lastname = "Mustermann";
                    example->phonenumber = "017623262367345743";
                    example->birthDate = "2000-05-21";
                    example->secretQuestion1 = "Was ist dein Lieblingsessen?";
                    example->secretQuestion2 = "Wie hieß deiner erster Hund?";
                    example->secretQuestion3 = "Wie hieß deine erste Katze?" ;
                    example->secretAnswer1 = "Nudeln";
                    example->secretAnswer2 = "Fritz" ;
                    example->secretAnswer3 = "Galia" ;

                    info->body.addExample("Registering Max Mustermann", example);

                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully registered the user. The response contains the user ID.";

                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: Missing or invalid required fields such as email, password, nickname, etc.";

                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An error occurred during the registration process.";

                    info->addTag("User");
                }
                ENDPOINT_ASYNC("POST", "/api/v1/user/register", RegisterUser)
                {
                    ENDPOINT_ASYNC_INIT(RegisterUser);

                    Action act() override
                    {
                        return request->readBodyToDtoAsync<Object<microTube::dto::UserRegistrationDTO>>(controller->m_objectMapper).callbackTo(&RegisterUser::onBodyRead);
                    }

                    Action onBodyRead(const Object<microTube::dto::UserRegistrationDTO>&user)
                    {
                        OATPP_ASSERT_HTTP(user
                            && !user->email->empty()
                            && user->email->find("@", 0) != user->email->npos
                            && user->email->find(".", 0) != user->email->npos, Status::CODE_400, "Email empty or invalid");

                        OATPP_ASSERT_HTTP(user && !user->password->empty()       , Status::CODE_400, "Password is required");
                        OATPP_ASSERT_HTTP(user && !user->nickname->empty()       , Status::CODE_400, "Nickname is required");
                        OATPP_ASSERT_HTTP(user && !user->firstname->empty()      , Status::CODE_400, "Firstname is required");
                        OATPP_ASSERT_HTTP(user && !user->lastname->empty()       , Status::CODE_400, "Lastname is required and must be a valid number");
                        OATPP_ASSERT_HTTP(user && !user->phonenumber->empty()    , Status::CODE_400, "Phone number is required");
                        OATPP_ASSERT_HTTP(user && !user->birthDate->empty()      , Status::CODE_400, "Birthdate is required");
                        OATPP_ASSERT_HTTP(user && !user->secretQuestion1->empty(), Status::CODE_400, "Secret question 1 is required");
                        OATPP_ASSERT_HTTP(user && !user->secretQuestion2->empty(), Status::CODE_400, "Secret question 2 is required");
                        OATPP_ASSERT_HTTP(user && !user->secretQuestion3->empty(), Status::CODE_400, "Secret question 3 is required");
                        OATPP_ASSERT_HTTP(user && !user->secretAnswer1->empty()  , Status::CODE_400, "Answer to secret question 1 is required");
                        OATPP_ASSERT_HTTP(user && !user->secretAnswer2->empty()  , Status::CODE_400, "Answer to secret question 2 is required");
                        OATPP_ASSERT_HTTP(user && !user->secretAnswer3->empty()  , Status::CODE_400, "Answer to secret question 3 is required");

                        auto dbResultBundle = controller->m_database->registerUser(user);

                        const bool& dbTransactionResult = dbResultBundle->isSuccess();
                        if (!dbTransactionResult)
                        {
                            const String& ErrMsg = dbResultBundle->getErrorMessage();
                            OATPP_ASSERT_HTTP(dbTransactionResult, Status::CODE_500, ErrMsg->c_str());
                        }

                        const auto& ResultList = dbResultBundle->fetch<oatpp::List<oatpp::List<UInt64>>>();

                        assert(ResultList->size() == 1);
                        OATPP_ASSERT_HTTP(ResultList->size() == 1, Status::CODE_500, "More than one user created");

                        const String& UserId = oatpp::utils::Conversion::uint64ToStr(ResultList[0][0]);

                        return _return(controller->createResponse(Status::CODE_200, UserId->c_str()));
                    }
                };

                ENDPOINT_INFO(LoginUser)
                {
                    info->summary = "Log in a user";
                    info->description = "This endpoint allows a user to log in by providing their email and password. Upon successful authentication, a bearer token is returned.";

                    // Request body documentation
                    info->addConsumes<Object<dto::UserLoginDTO>>("application/json", "The credentials of the user trying to log in.");

                    auto example = dto::UserLoginDTO::createShared();
                    example->email = "max.mustermann@muster.com";
                    example->password = "12345";

                    info->body.addExample("Logging in Max Mustermann", example);

                    // Response documentation for a successful login (Bearer token)
                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully authenticated the user. The response contains the bearer token.";

                    // Response documentation for bad request (missing fields)
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: Missing or invalid email or password.";

                    // Response documentation for user not found
                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "User not found: The provided email is not associated with any registered user.";

                    // Response documentation for wrong password
                    info->addResponse<String>(Status::CODE_401, "text/plain")
                        .description = "Unauthorized: The provided password is incorrect.";

                    // Response documentation for internal server error
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred during the authentication process.";

                    // Tag for grouping endpoints related to users
                    info->addTag("User");
                }
                ENDPOINT_ASYNC("POST", "/api/v1/user/login", LoginUser)
                {
                    ENDPOINT_ASYNC_INIT(LoginUser);

                    Action act() override
                    {
                        return request->readBodyToDtoAsync<Object<dto::UserLoginDTO>>(controller->m_objectMapper).callbackTo(&LoginUser::ProcessLogin);
                    }

                    Action ProcessLogin(const Object<dto::UserLoginDTO>&dto)
                    {
                        OATPP_ASSERT_HTTP(dto->email
                            && dto->password
                            && !dto->email->empty()
                            && !dto->password->empty(), Status::CODE_400, "Missing email or password");

                        m_userLoginData = dto;

                        return this->VerifyEmail();
                    }

                    Action VerifyEmail()
                    {
                        auto dbResultUserId = controller->m_database->getUserIdByEmail(this->m_userLoginData->email);

                        OATPP_ASSERT_HTTP(dbResultUserId->isSuccess(), Status::CODE_500, "Internal Server error");

                        const auto DbUserIdResultList = dbResultUserId->fetch<oatpp::List<oatpp::List<UInt64>>>();

                        OATPP_ASSERT_HTTP(!DbUserIdResultList->empty()
                            && !DbUserIdResultList[0]->empty(), Status::CODE_404, "User Unknown");

                        m_userId = DbUserIdResultList[0][0];

                        return this->VerifyPassword();
                    }

                    Action VerifyPassword()
                    {
                        auto dbResultUserPasswd = controller->m_database->getUserByIdAndPassword(m_userId, this->m_userLoginData->password);

                        OATPP_ASSERT_HTTP(dbResultUserPasswd->isSuccess(), Status::CODE_500, "Internal Server error");

                        const auto DbUserPswdResultList = dbResultUserPasswd->fetch<oatpp::List<Object<dto::UserDatabaseEntry>>>();

                        OATPP_ASSERT_HTTP(!DbUserPswdResultList->empty(), Status::CODE_401, "Wrong password");
                        OATPP_ASSERT_HTTP(DbUserPswdResultList->size() == 1, Status::CODE_500, "Internal Server error");

                        this->m_userData = DbUserPswdResultList[0];

                        OATPP_ASSERT_HTTP(m_userLoginData && m_userData, Status::CODE_500, "Internal Server error");

                        return this->GetOrCreateBearerTokenInDatabase();
                    }

                    Action GetOrCreateBearerTokenInDatabase()
                    {
                        String token;

                        auto dbResult = controller->m_database->getValidTokens(this->m_userId);

                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto resultList = dbResult->fetch<List<List<String>>>();

                        if (!resultList->empty() && !resultList[0]->empty())
                        {
                            token = resultList[0][0];
                            auto dbResult = controller->m_database->extendTokenValidity(token);
                            OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        }
                        else
                        {
                            token = this->generateBearerToken();
                            auto dbResult = controller->m_database->insertNewToken(token, this->m_userId);
                            OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        }
                        return _return(controller->createResponse(Status::CODE_200, token));
                    }

                    static inline oatpp::String generateBearerToken(const size_t length = 50)
                    {
                        const std::string characters{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" };
                        std::random_device rd;
                        std::mt19937 generator(rd());
                        std::uniform_int_distribution<size_t> distribution(0, characters.size() - 1);

                        std::string token{};

                        for (size_t i = 0; i < length; ++i)
                            token += characters[distribution(generator)];

                        return token;
                    }

                private:
                    UInt64                        m_userId;
                    Object<dto::UserLoginDTO>     m_userLoginData;
                    Object<dto::UserDatabaseEntry> m_userData;
                };

                ENDPOINT_INFO(GetInfo)
                {
                    info->summary = "Get information about a user";
                    info->description = "This endpoint allows retrieving user information by user ID. It requires a valid Bearer token for authentication. If no `userId` is provided as a query parameter, the information of the authenticated user is returned. Admin users can request information for other users.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";

                    info->headers.add<String>("userId").description = "The ID of the user whose information is being requested. If not provided, the requesting user's info is returned.";

                    info->addResponse<Object<dto::UserInfoDTO>>(Status::CODE_200, "application/json")
                        .description = "Successfully retrieved the user information.";
                    info->addResponse<String>(Status::CODE_401, "text/plain")
                        .description = "Unauthorized: The requester is not an admin and the requested user ID does not match the authenticated user.";
                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Not Found: The user with the specified ID could not be found.";
                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An unexpected error occurred while retrieving the user information.";

                    // Tag for grouping endpoints related to user information
                    info->addTag("User");
                }
                ENDPOINT_ASYNC("GET", "/api/v1/user", GetInfo)
                {
                    ENDPOINT_ASYNC_INIT(GetInfo);

                    Action act() override
                    {
                        this->m_IdTokenPair = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                        String userId = request->getQueryParameter("userId");
                        if (!userId || userId->empty())
                        {
                            userId = oatpp::utils::Conversion::uint64ToStr(m_IdTokenPair->id);
                        }

                        m_requesterIsAdmin = false;
                        if (m_IdTokenPair->id != m_requestedId)
                        {
                            m_requesterIsAdmin = controller->m_database->isUserAdmin(m_IdTokenPair->id);

                            OATPP_ASSERT_HTTP(m_requesterIsAdmin, Status::CODE_401, "Unauthorized");
                        }

                        return this->GetRequestedUser();
                    }

                    Action GetRequestedUser()
                    {
                        auto dbResult = controller->m_database->getUserInfoById(m_requestedId);

                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto dbResultList = dbResult->fetch<List<Object<dto::UserDatabaseEntry>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "Unknown requested user");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        if (m_requesterIsAdmin)
                            return _return(controller->createDtoResponse(Status::CODE_200, dbResultList[0]));

                        Object<dto::UserInfoDTO> ret = dbResultList[0]->operator oatpp::data::type::DTOWrapper<microTube::dto::UserInfoDTO>();

                        return _return(controller->createDtoResponse(Status::CODE_200, ret));
                    }

                private:
                    Object<dto::UserTokenPairDTO> m_IdTokenPair;
                    UInt64                        m_requestedId;
                    bool                          m_requesterIsAdmin;
                };

                ENDPOINT_INFO(UpdateInfo)
                {
                    info->summary = "Update user information";
                    info->description = "This endpoint allows updating a user's information. A valid Bearer token is required for authentication. If the `userId` query parameter is provided, it allows an admin to update another user's information. If no `userId` is provided, the authenticated user can update their own information.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";

                    info->queryParams.add<String>("userId").description = "The ID of the user whose information is being updated. If not provided, the authenticated user's information will be updated.";

                    info->addConsumes<Object<dto::UserDatabaseEntry>>("application/json", "The user information to update");

                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully updated user information.";

                    info->addResponse<String>(Status::CODE_401, "text/plain")
                        .description = "Unauthorized: The requester is not an admin and does not have permission to update the specified user's information.";

                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: The request is missing or has an invalid Bearer token.";

                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Not Found: The user with the specified ID could not be found.";

                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An error occurred while updating the user information.";

                    info->addTag("User");
                }
                ENDPOINT_ASYNC("PATCH", "/api/v1/user", UpdateInfo)
                {
                    ENDPOINT_ASYNC_INIT(UpdateInfo);

                    Action act()
                    {
                        const String authHeader = request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION);
                        OATPP_ASSERT_HTTP(authHeader && !authHeader->empty(), Status::CODE_400, "Authentification required");

                        this->requesterIsAdmin = false;

                        auto dbResult = controller->m_database->isTokenValid(authHeader->substr(7));
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto dbResultList = dbResult->fetch<List<Object<dto::UserTokenPairDTO>>>();

                        OATPP_ASSERT_HTTP(!dbResultList->empty(), Status::CODE_404, "Unknown token");
                        OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                        this->requesterIdTokenPair = dbResultList[0];

                        return request->readBodyToDtoAsync<Object<dto::UserDatabaseEntry>>(controller->m_objectMapper).callbackTo(&UpdateInfo::onDtoObtained);
                    }

                    Action onDtoObtained(const Object<dto::UserDatabaseEntry>& dto)
                    {
                        auto UpdateUserIdStr = request->getQueryParameter("userId");

                        UInt64 UpdateId{};

                        if (UpdateUserIdStr)
                        {
                            UpdateId = oatpp::utils::Conversion::strToUInt64(UpdateUserIdStr->c_str());

                            if (UpdateId != requesterIdTokenPair->id)
                            {
                                this->checkAdminPrivilege();
                                OATPP_ASSERT_HTTP(requesterIsAdmin, Status::CODE_401, "Unauthorized");
                            }
                        }
                        else
                            UpdateId = requesterIdTokenPair->id;

                        auto UpdateDto = dto;

                        UpdateDto->joinDate.resetPtr();

                        if (!requesterIsAdmin)
                            UpdateDto->isAdmin.resetPtr();

                        auto dbResult = this->controller->m_database->updateUserInfo(UpdateDto, UpdateId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        return _return(controller->createResponse(Status::CODE_200));
                    }

                    void checkAdminPrivilege()
                    {
                        this->requesterIsAdmin = controller->m_database->isUserAdmin(requesterIdTokenPair->id);
                    }

                private:
                    bool requesterIsAdmin;
                    Object<dto::UserTokenPairDTO> requesterIdTokenPair;
                };

                ENDPOINT_INFO(GetProfilePicture)
                {
                    info->summary = "Retrieve a user's profile picture";
                    info->description = "This endpoint allows retrieving a user's profile picture. A valid Bearer token is required for authentication. Optionally, a `userId` query parameter can be provided to retrieve the profile picture of another user. If no `userId` is provided, the authenticated user's profile picture will be returned.";

                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";

                    info->queryParams.add<String>("userId").description = "The ID of the user whose profile picture is being requested. If not provided, the authenticated user's profile picture will be returned.";

                    info->addResponse<oatpp::String>(Status::CODE_200, "image/jpeg")
                        .description = "Successfully retrieved the profile picture.";

                    info->addResponse<String>(Status::CODE_404, "text/plain")
                        .description = "Not Found: The requested profile picture could not be found.";

                    info->addResponse<String>(Status::CODE_500, "text/plain")
                        .description = "Internal Server Error: An error occurred while retrieving the profile picture.";

                    info->addTag("User");
                }
                ENDPOINT_ASYNC("GET", "api/v1/user/profile/picture", GetProfilePicture)
                {
                    ENDPOINT_ASYNC_INIT(GetProfilePicture);

                    Action act() override
                    {
                        oatpp::String userId = request->getQueryParameter("userId");
                        if (userId->empty())
                            userId = oatpp::utils::Conversion::uint64ToStr(controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION))->id);

                        oatpp::String profilePicturePath = USER_PROFILE_PICTURE_FOLDER + userId + "/profilepicture.jpg";

                        OATPP_ASSERT_HTTP(std::filesystem::exists(USER_PROFILE_PICTURE_FOLDER), Status::CODE_500, "Internal Server Error");
                        OATPP_ASSERT_HTTP(std::filesystem::exists(profilePicturePath->c_str()), Status::CODE_404, "Profile picture not found");

                        auto fileData = oatpp::String::loadFromFile(profilePicturePath->c_str());
                        return _return(controller->createResponse(Status::CODE_200, fileData));
                    }
                };

                ENDPOINT_INFO(UploadProfilePicture)
                {
                    info->summary = "Upload a profile picture";
                    info->description = "This endpoint allows users to upload their profile picture. The request must contain an image file in the JPEG format. A valid Bearer token must be provided for authentication.";

                    // Header documentation for Authorization (Bearer Token)
                    info->headers.add<String>("Authorization").description = "Bearer token used for authentication.";

                    // Content-Type header for file upload
                    info->headers.add<String>("Content-Type").description = "The content type of the file being uploaded. Expected value: image/jpeg.";

                    // Response for successful file upload
                    info->addResponse<String>(Status::CODE_200, "text/plain")
                        .description = "Successfully uploaded the profile picture.";

                    // Response for bad request (no content in the body)
                    info->addResponse<String>(Status::CODE_400, "text/plain")
                        .description = "Bad Request: No content in the request body.";

                    // Response for unsupported media type (wrong file type)
                    info->addResponse<String>(Status::CODE_415, "text/plain")
                        .description = "Unsupported Media Type: The file must be of type image/jpeg.";

                    // Response for unauthorized access (invalid Bearer token)
                    info->addResponse<String>(Status::CODE_401, "text/plain")
                        .description = "Unauthorized: Invalid Bearer token or missing authorization.";

                    // Tag for grouping endpoints related to user profile
                    info->addTag("User");
                }
                ENDPOINT_ASYNC("POST", "api/v1/user/profile/picture", UploadProfilePicture)
                {
                    ENDPOINT_ASYNC_INIT(UploadProfilePicture);

                    Action act() override
                    {
                        const auto& body = request->readBodyToString();

                        OATPP_ASSERT_HTTP(!body->empty(), Status::CODE_400, "No content in request body");
                        OATPP_ASSERT_HTTP(request->getHeader("Content-Type") == "image/jpeg", Status::CODE_415, "Invalid file type, expected image/jpeg");

                        const auto& IdTokenPair = controller->m_database->checkBearerThrowIfInvalid(request->getHeader(oatpp::web::protocol::http::Header::AUTHORIZATION));

                        const oatpp::String& profileFolderPath = USER_PROFILE_PICTURE_FOLDER + oatpp::utils::Conversion::uint64ToStr(IdTokenPair->id);
                        const oatpp::String& profilePicturePath = profileFolderPath + "/profilepicture.jpg";
                                       

                        std::filesystem::create_directory(USER_PROFILE_PICTURE_FOLDER);
                        std::filesystem::create_directory(profileFolderPath->c_str());

                        body.saveToFile(profilePicturePath->c_str());

                        return _return(controller->createResponse(Status::CODE_200, "Profile picture uploaded successfully"));
                    }
                };

            private:
                OATPP_COMPONENT(std::shared_ptr<ObjectMapper>                        , m_objectMapper);
                OATPP_COMPONENT(std::shared_ptr<microTube::component::DatabaseClient>, m_database);
            };

#include OATPP_CODEGEN_END(ApiController)

    } // namespace apicontroller
} // namespace microTube
