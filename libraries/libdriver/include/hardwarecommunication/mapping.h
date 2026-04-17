//
// Created by Max Tyson on 16/04/2026.
//

#ifndef LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H
#define LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H

namespace LibDriver::HardwareCommunication
{

	typedef struct HardwareMapping
	{
		uint32_t	port_base;
		size_t		port_length;

		uintptr_t	memio_base;
		size_t		memio_length;

		uint32_t	interrupts_base;
		size_t		interrupts_length;

	} hardware_mapping_t;

}

#endif //LIBDRIVER_HARDWARECOMMUNICATION_ATTACHMENT_H