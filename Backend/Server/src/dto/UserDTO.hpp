#ifndef USERDTO_HPP
#define USERDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserDTO : public oatpp::DTO
        {
            using UInt64 = oatpp::UInt64;
            using String = oatpp::String;
            using UInt32 = oatpp::UInt32;
            using UInt8  = oatpp::UInt8 ;

            DTO_INIT(UserDTO, DTO);

            DTO_FIELD(String, nickname);
            DTO_FIELD_INFO(nickname) {
                info->description = "Nickname of the user";
            }

            DTO_FIELD(String, firstName);
            DTO_FIELD_INFO(firstName) {
                info->description = "First name of the user";
            }            
            
            DTO_FIELD(String, lastName);
            DTO_FIELD_INFO(lastName) {
                info->description = "Last name of the user";
            }

            DTO_FIELD(String, email);
            DTO_FIELD_INFO(email) {
                info->description = "Email address of the user";
            }

            DTO_FIELD(UInt64, userId);
            DTO_FIELD_INFO(userId) {
                info->description = "The user id";
            }

            DTO_FIELD(String, registrationDate);
            DTO_FIELD_INFO(registrationDate) {
                info->description = "The date the user registered";
                info->pattern = "2012-04-23T18:25:43.511Z";
            }

        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // STATUSDTO_HPP