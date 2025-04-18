//
// Created by Max Tyson on 18/04/2025.
//

#ifndef MAXOS_DRIVERS_IDE_H
#define MAXOS_DRIVERS_IDE_H

#include <drivers/disk/ata.h>
#include <stdint.h>
#include <hardwarecommunication/pci.h>


namespace MaxOS{

    namespace drivers{

        namespace disk{

            /**
            * * @class IntegratedDriveElectronicsController
            * @brief Driver for the IDE controller, handles the creation and management of the IDE devices
            */
            class IntegratedDriveElectronicsController : public Driver{

                AdvancedTechnologyAttachment primary_maser;
                AdvancedTechnologyAttachment primary_slave;

                AdvancedTechnologyAttachment secondary_maser;
                AdvancedTechnologyAttachment secondary_slave;


                public:
                    IntegratedDriveElectronicsController(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* device_descriptor);
                    ~IntegratedDriveElectronicsController();

                    void initialise() override;

                    string vendor_name() final;
                    string device_name() final;


            };
        }
    }
}


#endif //MAXOS_DRIVERS_IDE_H
