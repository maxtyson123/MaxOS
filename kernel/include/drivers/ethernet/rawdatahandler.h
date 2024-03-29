//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H
#define MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H

#include <stdint.h>

namespace MaxOS {

    namespace drivers {

        namespace ethernet {

            /**
             * @class RawDataHandler
             * @brief Handles raw data received from the ethernet controller
             */
            class RawDataHandler
            {
                public:
                    RawDataHandler();
                    ~RawDataHandler();
                    virtual bool HandleRawData(uint8_t* buffer, uint32_t size);
            };


        }

    }

}

#endif //MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H
