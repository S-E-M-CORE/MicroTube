#ifndef USERDTO_HPP
#define USERDTO_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserRegistrationDTO : public oatpp::DTO
        {
            DTO_INIT(UserRegistrationDTO, DTO);

            DTO_FIELD(String, email);
            DTO_FIELD_INFO(email)
            {
                info->description = "Email of the user";
            }

            DTO_FIELD(String, password);
            DTO_FIELD_INFO(password)
            {
                info->description = "Password of the user (will be hashed before passed to databse)";
            }            
            
            DTO_FIELD(String, nickname);
            DTO_FIELD_INFO(nickname)
            {
                info->description = "Nickname of the user";
            }

            DTO_FIELD(String, firstname);
            DTO_FIELD_INFO(firstname)
            {
                info->description = "firstname of the user";
            }

            DTO_FIELD(String, lastname);
            DTO_FIELD_INFO(lastname)
            {
                info->description = "lastname of the user";
            }

            DTO_FIELD(String, phonenumber);
            DTO_FIELD_INFO(phonenumber)
            {
                info->description = "phone number of the user";
            }

            DTO_FIELD(String, birthDate);
            DTO_FIELD_INFO(birthDate)
            {
                info->description = "Birthday of the user";
            }

            DTO_FIELD(String, secretQuestion1);
            DTO_FIELD_INFO(secretQuestion1)
            {
                info->description = "Secret question for password recovery";
            }

            DTO_FIELD(String, secretQuestion2);
            DTO_FIELD_INFO(secretQuestion2)
            {
                info->description = "Secret question for password recovery";
            }

            DTO_FIELD(String, secretQuestion3);
            DTO_FIELD_INFO(secretQuestion3)
            {
                info->description = "Secret question for password recovery";
            }

            DTO_FIELD(String, secretAnswer1);
            DTO_FIELD_INFO(secretAnswer1)
            {
                info->description = "Answer to secret question nr. 1";
            }

            DTO_FIELD(String, secretAnswer2);
            DTO_FIELD_INFO(secretAnswer2)
            {
                info->description = "Answer to secret question nr. 2";
            }

            DTO_FIELD(String, secretAnswer3);
            DTO_FIELD_INFO(secretAnswer3)
            {
                info->description = "Answer to secret question nr. 3";
            }
        };
#include OATPP_CODEGEN_END(DTO)
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserInfoDTO : public oatpp::DTO
        {
            DTO_INIT(UserInfoDTO, DTO);

            DTO_FIELD(String, email);
            DTO_FIELD_INFO(email)
            {
                info->description = "Email of the user";
            }

            DTO_FIELD(String, nickname);
            DTO_FIELD_INFO(nickname)
            {
                info->description = "Nickname of the user";
            }

            DTO_FIELD(String, firstname);
            DTO_FIELD_INFO(firstname)
            {
                info->description = "firstname of the user";
            }

            DTO_FIELD(String, lastname);
            DTO_FIELD_INFO(lastname)
            {
                info->description = "lastname of the user";
            }

            DTO_FIELD(String, phonenumber);
            DTO_FIELD_INFO(phonenumber)
            {
                info->description = "phone number of the user";
            }

            DTO_FIELD(String, birthDate);
            DTO_FIELD_INFO(birthDate)
            {
                info->description = "Birthday of the user";
            }
        };
#include OATPP_CODEGEN_END(DTO)
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserLoginDTO : public oatpp::DTO
        {
            DTO_INIT(UserLoginDTO, DTO);

            DTO_FIELD(String, email);
            DTO_FIELD_INFO(email)
            {
                info->description = "Email of the user";
            }

            DTO_FIELD(String, password);
            DTO_FIELD_INFO(password)
            {
                info->description = "Password of the user";
            }
        };

#include OATPP_CODEGEN_END(DTO)
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserDatabaseEntry : public oatpp::DTO
        {
            DTO_INIT(UserDatabaseEntry, DTO);

            DTO_FIELD(Int64, id);
            DTO_FIELD_INFO(id)
            {
                info->description = "User id";
            }

            DTO_FIELD(String, email);
            DTO_FIELD_INFO(email)
            {
                info->description = "Email of the user";
            }

            DTO_FIELD(String, password);
            DTO_FIELD_INFO(password)
            {
                info->description = "Password of the user";
            }

            DTO_FIELD(String, nickname);
            DTO_FIELD_INFO(nickname)
            {
                info->description = "Nickname of the user";
            }

            DTO_FIELD(String, firstname);
            DTO_FIELD_INFO(firstname)
            {
                info->description = "First name of the user";
            }

            DTO_FIELD(String, lastname);
            DTO_FIELD_INFO(lastname)
            {
                info->description = "Last name of the user";
            }

            DTO_FIELD(String, phonenumber);
            DTO_FIELD_INFO(phonenumber)
            {
                info->description = "Phone number of the user";
            }

            DTO_FIELD(String, birthDate);
            DTO_FIELD_INFO(birthDate)
            {
                info->description = "Birth date of the user";
            }

            DTO_FIELD(String, joinDate);
            DTO_FIELD_INFO(joinDate)
            {
                info->description = "Join date of the user";
            }

            DTO_FIELD(String, secretQuestion1);
            DTO_FIELD_INFO(secretQuestion1)
            {
                info->description = "Secret question 1 for account recovery";
            }

            DTO_FIELD(String, secretQuestion2);
            DTO_FIELD_INFO(secretQuestion2)
            {
                info->description = "Secret question 2 for account recovery";
            }

            DTO_FIELD(String, secretQuestion3);
            DTO_FIELD_INFO(secretQuestion3)
            {
                info->description = "Secret question 3 for account recovery";
            }

            DTO_FIELD(String, secretAnswer1);
            DTO_FIELD_INFO(secretAnswer1)
            {
                info->description = "Answer to secret question 1";
            }

            DTO_FIELD(String, secretAnswer2);
            DTO_FIELD_INFO(secretAnswer2)
            {
                info->description = "Answer to secret question 2";
            }

            DTO_FIELD(String, secretAnswer3);
            DTO_FIELD_INFO(secretAnswer3)
            {
                info->description = "Answer to secret question 3";
            }

            DTO_FIELD(Boolean, isAdmin);
            DTO_FIELD_INFO(isAdmin)
            {
                info->description = "Flag indicating if the user is an administrator";
            }

            DTO_FIELD(Boolean, isActive);
            DTO_FIELD_INFO(isActive)
            {
                info->description = "Flag indicating if the user is active";
            }

            operator UserInfoDTO::Wrapper()
            {
                auto ret = UserInfoDTO::createShared();

                ret->email = this->email;
                ret->nickname = this->nickname;
                ret->firstname = this->firstname;
                ret->lastname = this->lastname;
                ret->phonenumber = this->phonenumber;
                ret->birthDate = this->birthDate;

                return ret;
            }

        };

#include OATPP_CODEGEN_END(DTO)
#include OATPP_CODEGEN_BEGIN(DTO)

        class UserTokenPairDTO : public oatpp::DTO
        {
            DTO_INIT(UserTokenPairDTO, DTO);

            DTO_FIELD(String, token);
            DTO_FIELD_INFO(token)
            {
                info->description = "Bearer token";
            }

            DTO_FIELD(UInt64, id);
            DTO_FIELD_INFO(id)
            {
                info->description = "User id";
            }
        };
#include OATPP_CODEGEN_END(DTO)
#include OATPP_CODEGEN_BEGIN(DTO)

        class ChannelInfoDTO : public oatpp::DTO
        {
            DTO_INIT(ChannelInfoDTO, DTO);

            DTO_FIELD(String, nickname);
            DTO_FIELD_INFO(nickname)
            {
                info->description = "nickname";
            }

            DTO_FIELD(UInt64, id);
            DTO_FIELD_INFO(id)
            {
                info->description = "User id to the channel";
            }

            DTO_FIELD(String, channelDescription);
            DTO_FIELD_INFO(channelDescription)
            {
                info->description = "Description of the channel";
            }
        };
#include OATPP_CODEGEN_END(DTO)
    } // namespace dto
} // namespace microTube
#endif // STATUSDTO_HPP