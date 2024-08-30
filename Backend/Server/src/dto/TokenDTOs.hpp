#ifndef TOKENSDTO_HPP
#define TOKENSDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)

        class BearerTokenDTO : public oatpp::DTO
        {
            using UInt64 = oatpp::UInt64;
            using String = oatpp::String;

            DTO_INIT(VideoInfoDTO, DTO);

            DTO_FIELD(String, bearerToken);
            DTO_FIELD_INFO(token) {
                info->description = "The requested token";
            }

            DTO_FIELD(String, username);
            DTO_FIELD_INFO(username) {
                info->description = "The username the token is valid for";
            }

            DTO_FIELD(UInt64, userId);
            DTO_FIELD_INFO(userId) {
                info->description = "The user id the token is valid for";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // VIDEOINFODTO_HPP