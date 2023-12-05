//
// Created by 98max on 24/10/2022.
//

#ifndef MAXOS_DRIVERS_ATA_H
#define MAXOS_DRIVERS_ATA_H

#include <stdint.h>
#include <common/outputStream.h>
#include <hardwarecommunication/port.h>

namespace maxOS{

    namespace drivers{

        class AdvancedTechnologyAttachment{

            protected:
                hardwarecommunication::Port16Bit dataPort;
                hardwarecommunication::Port8Bit errorPort;
                hardwarecommunication::Port8Bit sectorCountPort;
                hardwarecommunication::Port8Bit LBAlowPort;
                hardwarecommunication::Port8Bit LBAmidPort;
                hardwarecommunication::Port8Bit LBAHiPort;
                hardwarecommunication::Port8Bit devicePort;
                hardwarecommunication::Port8Bit commandPort;
                hardwarecommunication::Port8Bit controlPort;
                bool master;
                uint16_t bytesPerSector;

                common::OutputStream* ataMessageStream;
            public:
                AdvancedTechnologyAttachment(uint16_t portBase, bool master, common::OutputStream* ataMessageStream);
                ~AdvancedTechnologyAttachment();

                void Identify();
                void Read28(uint32_t sector, uint8_t* data, int count);
                void Write28(uint32_t sector, uint8_t* data, int count);
                void Flush();                                                                               //Flush Cache  //TODO: See also vid 19 24:20

                //TODO: Make into driver class


        };

    }

}

#endif //MAXOS_DRIVERS_ATA_H
