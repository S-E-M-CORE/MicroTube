#ifndef VIDEOINFOTRANSFERDTO_HPP
#define VIDEOINFOTRANSFERDTO_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"
#include "InfoJsonDTO.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)

        /*
        DTO for video infos, which also includes database infos
        */
        class VideoInfoTransferDTO : public oatpp::DTO
        {
            using UInt64 = oatpp::UInt64;
            using String = oatpp::String;

            DTO_INIT(VideoInfoTransferDTO, DTO);

            DTO_FIELD(oatpp::Object<microTube::dto::InfoJsonDTO>, jsonData);
            DTO_FIELD_INFO(jsonData)
            {
                info->description = "Information from within the json file";
            }

            DTO_FIELD(UInt64, likeCount);
            DTO_FIELD_INFO(likeCount)
            {
                info->description = "Number of likes on the video";
            }

            DTO_FIELD(UInt64, dislikeCount);
            DTO_FIELD_INFO(dislikeCount)
            {
                info->description = "Number of dislikes on the video";
            }

            DTO_FIELD(UInt64, uniqueViews);
            DTO_FIELD_INFO(uniqueViews)
            {
                info->description = "Number of registered users who have seen the video";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // VIDEOINFOTRANSFERDTO_HPP