#ifndef STATUSDTO_HPP
#define STATUSDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace microTube
{
    namespace dto
    {
#include OATPP_CODEGEN_BEGIN(DTO)
        //  ____  _        _             ____  _        
        // / ___|| |_ __ _| |_ _   _ ___|  _ \| |_ ___  
        // \___ \| __/ _` | __| | | / __| | | | __/ _ \ 
        //  ___) | || (_| | |_| |_| \__ \ |_| | || (_) |
        // |____/ \__\__,_|\__|\__,_|___/____/ \__\___/ 
        /**
        * @brief Data transfer object (DTO) class for representing status information.
        */
        class StatusDto : public oatpp::DTO
        {
            using Int32 = oatpp::Int32;
            using String = oatpp::String;

            DTO_INIT(StatusDto, DTO); 

            DTO_FIELD(Int32, code); 
            DTO_FIELD_INFO(code) {  
                info->description = "Status code";
            }

            DTO_FIELD(String, status);
            DTO_FIELD_INFO(status) {  
                info->description = "Short status text";
            }

            DTO_FIELD(String, message);
            DTO_FIELD_INFO(message) {  
                info->description = "Verbose message";
            }
        };
#include OATPP_CODEGEN_END(DTO)

    } // namespace dto
} // namespace microTube
#endif // STATUSDTO_HPP