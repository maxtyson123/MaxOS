/**
 * @file ethernetframe.cpp
 * @brief Implementation of Ethernet Frame Handler and Payload Handler
 *
 * @date 11th May 2022
 * @author Max Tyson
 */

#include <net/ethernetframe.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::net;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::memory;

/**
 * @brief Construct a new Ether Frame Payload Handler object
 *
 * @param frame_handler the handler for the ethernet frame
 * @param handled_type the type of the protocol, which will be handled by this handler
 */
EthernetFramePayloadHandler::EthernetFramePayloadHandler(EthernetFrameHandler* frame_handler, uint16_t handled_type) {

	this->handled_type = handled_type;
	this->frame_handler = frame_handler;
	frame_handler->connect_handler(this);


}

/**
 * @brief Destroy the EtherFrameHandler:: EtherFrameHandler object, Removes it from the handler list
 *
 */
EthernetFramePayloadHandler::~EthernetFramePayloadHandler() = default;


/**
 * @brief Handle the received ethernet frame payload
 *
 * @param ethernetframe_payload the payload of the ethernet frame
 * @param size the size of the payload
 *
 * @return True if the data is to be sent back, false otherwise
 */
bool EthernetFramePayloadHandler::handle_ethernetframe_payload(uint8_t* ethernetframe_payload, uint32_t size) {

	//By default, don't handle it, will be handled in the override
	return false;

}

/**
 * @brief send an packet via the backend driver
 *
 * @param destination the destination MAC address
 * @param data the data to send
 * @param size the size of the payload
 */
void EthernetFramePayloadHandler::send(uint64_t destination, uint8_t* data, uint32_t size) {

	frame_handler->send_ethernet_frame(destination, handled_type, data, size);
}

/**
 * @brief Construct a new Ether Frame Handler object
 *
 * @param driver The backend ethernet driver
 * @param error_messages The output stream for error messages
 */
EthernetFrameHandler::EthernetFrameHandler(EthernetDriver* driver, OutputStream* error_messages)
		: EthernetDriverEventHandler() {

	this->ethernet_driver = driver;
	this->error_messages = error_messages;

	driver->connect_event_handler(this);

}

EthernetFrameHandler::~EthernetFrameHandler() = default;

/**
 * @brief Get the MAC address of this device
 *
 * @return MediaAccessControlAddress The MAC address
 */
drivers::ethernet::MediaAccessControlAddress EthernetFrameHandler::get_mac() {
	return ethernet_driver->get_media_access_control_address();
}


/**
 * @brief Handle the received packet
 *
 * @param buffer the buffer with the received data
 * @param size the size of the received data
 * @return True if the data is to be sent back, false otherwise
 *
 * @todo Future debugging me: the override is not being called in derived classes
 */
bool EthernetFrameHandler::data_received(uint8_t* buffer, uint32_t size) {

	error_messages->write("EFH: Data received\n");


	//Check if the size is big enough to contain an ethernet frame
	if(size < sizeof(EthernetFrameHeader))
		return false;

	//Convert to struct for easier use
	auto* frame = (EthernetFrameHeader*) buffer;
	bool send_back = false;

	//Only handle if it is for this device
	if(frame->destination_mac == 0xFFFFFFFFFFFF                                          //If it is a broadcast
	   || frame->destination_mac == ethernet_driver->get_media_access_control_address())      //If it is for this device
	{

		// Find the handler for the protocol
		Map<uint16_t, EthernetFramePayloadHandler*>::iterator handler_iterator = frame_handlers.find(frame->type);

		// If the handler is found
		if(handler_iterator != frame_handlers.end()) {

			//Handle the data
			error_messages->write("EFH: Handling ethernet frame payload\n");
			send_back = handler_iterator->second->handle_ethernetframe_payload(buffer + sizeof(EthernetFrameHeader),
			                                                                   size - sizeof(EthernetFrameHeader));
			error_messages->write("..DONE\n");

		} else {

			//If the handler is not found, print an error message
			error_messages->write("EFH: Unhandled ethernet frame type 0x");
			error_messages->write_hex(frame->type);
			error_messages->write("\n");

		}
	}

	//If the data is to be sent back again
	if(send_back) {

		error_messages->write("EFH: Sending back\n");

		frame->destination_mac = frame->source_mac;                             //Set the new destination to be the device the data was received from
		frame->source_mac = ethernet_driver->get_media_access_control_address();      //Set the new source to be this device's MAC address

	}

	//Return if the data is to be sent back
	return send_back;

}

/**
 * @brief connect a handler to the frame handler
 *
 * @param handler The handler to connect
 */
void EthernetFrameHandler::connect_handler(EthernetFramePayloadHandler* handler) {

	// Convert the protocol type to big endian
	uint16_t frame_type_be = ((handler->handled_type >> 8) & 0xFF) | ((handler->handled_type << 8) & 0xFF00);

	// Add the handler to the list
	frame_handlers.insert(frame_type_be, handler);

}

/**
 * @brief send an packet via the backend driver
 *
 * @param destination_mac the destination MAC address
 * @param frame_type the type of the protocol
 * @param data the data to send
 * @param size the size of the payload
 */
void EthernetFrameHandler::send_ethernet_frame(uint64_t destination_mac, uint16_t frame_type, uint8_t* data, uint32_t size) {

	error_messages->write("EFH: Sending frame...");

	//Allocate memory for the buffer
	auto* buffer = (uint8_t*) MemoryManager::kmalloc(size + sizeof(EthernetFrameHeader));
	auto* frame = (EthernetFrameHeader*) buffer;

	//Put data in the header
	frame->destination_mac = destination_mac;
	frame->source_mac = ethernet_driver->get_media_access_control_address();
	frame->type = (frame_type >> 8) | (frame_type << 8);                        //Convert to big endian

	//Copy the data
	for(uint8_t* src = data + size - 1, * dst = buffer + sizeof(EthernetFrameHeader) + size - 1; src >= data; --src, --dst)
		*dst = *src;

	//Send the data
	ethernet_driver->send(buffer, size + sizeof(EthernetFrameHeader));

	error_messages->write("Done\n");


	//Free the buffer
	MemoryManager::kfree(buffer);
}
