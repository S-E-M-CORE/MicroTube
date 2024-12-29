#ifndef VIDEOLISTDTO_HPP
#define VIDEOLISTDTO_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)

        class VideoListDto : public oatpp::DTO
        {
            DTO_INIT(VideoListDto, DTO);

            DTO_FIELD(oatpp::UInt64, size);
            DTO_FIELD_INFO(size)
            {
                info->description = "How many uuids were provided";
            }

            DTO_FIELD(oatpp::List<oatpp::String>, uuids);
            DTO_FIELD_INFO(uuids)
            {
                info->description = "The uuids to the videos";
            }

        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // VIDEOLISTDTO_HPP