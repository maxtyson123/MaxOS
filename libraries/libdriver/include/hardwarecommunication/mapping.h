//
// Created by Max Tyson on 16/04/2026.
//

#ifndef LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H
#define LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H

namespace LibDriver::HardwareCommunication
{

	enum class HardwareRangeType {
		NONE,
		PORT,
		MEMORY,
	};

	typedef struct HardwareRange {
		uint64_t address_base;
		size_t address_length;

		HardwareRangeType type;
	} hardware_range_t;

	constexpr size_t NUM_HARDWARE_RANGES = 6;		///< @todo may change when adding other enumerators

	typedef struct HardwareMapping{

		hardware_range_t ranges[NUM_HARDWARE_RANGES];
		uint32_t	irq;

	} hardware_mapping_t;

}

#endif //LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H