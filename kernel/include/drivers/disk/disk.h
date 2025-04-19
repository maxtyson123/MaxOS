//
// Created by 98max on 18/04/2025.
//

#ifndef MAXOS_DRIVERS_DISK_H
#define MAXOS_DRIVERS_DISK_H

#include <common/outputStream.h>
#include <drivers/driver.h>
#include <stdint.h>


namespace MaxOS{

    namespace drivers{

        namespace disk{

            /**
             * @class Disk
             * @brief Generic Disk, handles the reading and writing of data to the hard drive
             */
            class Disk : public Driver {

                public:
                    Disk();
                    ~Disk();

                    virtual void read(uint32_t sector, uint8_t* data_buffer, size_t amount);
                    virtual void write(uint32_t sector, const uint8_t* data, size_t count);
                    virtual void flush();

                    void activate() override;

                    string device_name() override;
                    string vendor_name() override;
            };
        }
    }
}

#endif //MAXOS_DRIVERS_DISK_H
