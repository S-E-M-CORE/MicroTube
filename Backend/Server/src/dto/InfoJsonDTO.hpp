#ifndef INFOJSONDTO_HPP
#define INFOJSONDTO_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

namespace microTube
{
    namespace dto
    {
        #include OATPP_CODEGEN_BEGIN(DTO)

        class InfoJsonDTO : public oatpp::DTO
        {
            DTO_INIT(InfoJsonDTO, DTO);

            DTO_FIELD(oatpp::String, uuid);
            DTO_FIELD_INFO(uuid)
            {
                info->description = "The file name on server side";
            }

            DTO_FIELD(oatpp::String, title);
            DTO_FIELD_INFO(title)
            {
                info->description = "Video title";
            }

            DTO_FIELD(oatpp::String, description);
            DTO_FIELD_INFO(description)
            {
                info->description = "Video description";
            }

            DTO_FIELD(oatpp::String, uploadDate);
            DTO_FIELD_INFO(uploadDate)
            {
                info->description = "When the video was first uploaded";
                info->pattern     = "2012-04-23";
            }

            DTO_FIELD(UInt64, userId);
            DTO_FIELD_INFO(userId)
            {
                info->description = "Id to the user the channel belonged to during upload";
            }

            DTO_FIELD(UInt64, channelId);
            DTO_FIELD_INFO(channelId)
            {
                info->description = "Id to the channel the video belongs to";
            }

            DTO_FIELD(List<String>, categories);
            DTO_FIELD_INFO(categories)
            {
                info->description = "Categories of the video";
            }
            DTO_FIELD(List<String>, tags);
            DTO_FIELD_INFO(tags)
            {
                info->description = "Tags of the video";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // INFOJSONDTO_HPP