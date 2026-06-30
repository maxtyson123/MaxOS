//
// Created by 98max on 30/06/2026.
//

#ifndef MAXOS_DEVICE_H
#define MAXOS_DEVICE_H

#include <cstdint>

#include <libdriver/driver.h>

namespace LibDriver {

	typedef struct DeviceModel {
		uint16_t vendor;
		uint16_t device;
		uint16_t revision;
	} device_model_t;

	typedef struct DeviceClass {
		uint8_t base;
		uint8_t sub;
		uint8_t interface;
	} device_class_t;

	typedef struct  DeviceIdentification {

		device_model_t model;
		device_class_t class_info;

		DriverType driver_type;

		MaxOS::string vendor_name;
		MaxOS::string device_name;
		MaxOS::string class_string;


	} device_identification_t;

	inline bool operator == (const device_identification_t& lhs, const device_identification_t& rhs) {
		return (lhs.model.vendor == rhs.model.vendor &&
				lhs.model.device == rhs.model.device &&
				lhs.model.revision == rhs.model.revision);
	}
}

#endif //MAXOS_DEVICE_H
