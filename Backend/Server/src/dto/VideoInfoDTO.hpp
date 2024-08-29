#ifndef VIDEOINFODTO_HPP
#define VIDEOINFODTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)

        class VideoInfoDTO : public oatpp::DTO
        {
            using UInt64 = oatpp::UInt64;
            using String = oatpp::String;

            DTO_INIT(VideoInfoDTO, DTO);

            DTO_FIELD(String, serverFileName);
            DTO_FIELD_INFO(serverFileName) {
                info->description = "The file name on server side";
            }

            DTO_FIELD(UInt64, likeCount);
            DTO_FIELD_INFO(likeCount) {
                info->description = "Number of likes on the video";
            }

            DTO_FIELD(UInt64, dislikeCount);
            DTO_FIELD_INFO(dislikeCount) {
                info->description = "Number of dislikes on the video";
            }

            DTO_FIELD(UInt64, viewCount);
            DTO_FIELD_INFO(viewCount) {
                info->description = "Number of views on the video";
            }

            DTO_FIELD(String, uploadDate);
            DTO_FIELD_INFO(uploadDate) {
                info->description = "When the video was first uploaded";
                info->pattern     = "2012-04-23T18:25:43.511Z";
            }

            DTO_FIELD(UInt64, channelId);
            DTO_FIELD_INFO(channelId) {
                info->description = "Id to the channel the video belongs to";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // VIDEOINFODTO_HPP