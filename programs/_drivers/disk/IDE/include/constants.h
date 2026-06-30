//
// Created by 98max on 30/06/2026.
//

#ifndef IDEDRIVER_CONSTANTS_H
#define IDEDRIVER_CONSTANTS_H

namespace IDEDriver {

	constexpr uint16_t LEGACY_PRIMARY_COMMAND_BASE = 0x1F0;
	constexpr uint16_t LEGACY_PRIMARY_CONTROL_BASE = 0x3F6;
	constexpr uint16_t LEGACY_SECONDARY_COMMAND_BASE = 0x170;
	constexpr uint16_t LEGACY_SECONDARY_CONTROL_BASE = 0x376;

	/**
	 * @brief  BitMask for the result of a command port
	 */
	enum class Status {

		ERROR			= 0x01,
		INDEX			= 0x02,
		CORRECTED_DATA	= 0x04,
		DATA_READY		= 0x08,
		SEEK_COMPLETE	= 0x10,
		WRITE_FAULT		= 0x20,
		READY			= 0x40,
		BUSY			= 0x80,

	};


};

#endif //IDEDRIVER_CONSTANTS_H
