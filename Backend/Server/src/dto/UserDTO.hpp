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

            DTO_INIT(UserDTO, DTO);

            DTO_FIELD(String, name);
            DTO_FIELD_INFO(code) {
                info->description = "Username";
            }

            DTO_FIELD(UInt64, channelId);
            DTO_FIELD_INFO(channelId) {
                info->description = "The corelating channel of the user";
            }

            DTO_FIELD(UInt32, videoUploadCount);
            DTO_FIELD_INFO(videoUploadCount) {
                info->description = "Count of videos the user has uploaded";
            }

            DTO_FIELD(UInt32, registrationDate);
            DTO_FIELD_INFO(registrationDate) {
                info->description = "The date the user registered";
                info->pattern = "2012-04-23T18:25:43.511Z";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // STATUSDTO_HPP