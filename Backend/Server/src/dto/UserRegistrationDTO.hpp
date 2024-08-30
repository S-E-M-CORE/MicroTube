#ifndef USERREGISTRATIONDTO_HPP
#define USERREGISTRATIONDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)
        class UserRegistrationDTO : public oatpp::DTO
        {
            using Int32 = oatpp::Int32;
            using String = oatpp::String;

            DTO_INIT(UserRegistrationDTO, DTO);

            DTO_FIELD(String, username);
            DTO_FIELD_INFO(username) {
                info->description = "The username";
            }

            DTO_FIELD(String, password);
            DTO_FIELD_INFO(password) {
                info->description = "The password of the user to be send as hash value";
            }

            DTO_FIELD(String, secretQuestion1);
            DTO_FIELD_INFO(secretQuestion1) {
                info->description = "Question for password recovery";
            }

            DTO_FIELD(String, secretQuestion2);
            DTO_FIELD_INFO(secretQuestion2) {
                info->description = "Question for password recovery";
            }

            DTO_FIELD(String, secretQuestion3);
            DTO_FIELD_INFO(secretQuestion3) {
                info->description = "Question for password recovery";
            }

            DTO_FIELD(String, question1Answer);
            DTO_FIELD_INFO(question1Answer) {
                info->description = "Answer to secret question nr. 1";
            }

            DTO_FIELD(String, question2Answer);
            DTO_FIELD_INFO(question2Answer) {
                info->description = "Answer to secret question nr. 2";
            }

            DTO_FIELD(String, question3Answer);
            DTO_FIELD_INFO(question3Answer) {
                info->description = "Answer to secret question nr. 3";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // USERREGISTRATIONDTO_HPP