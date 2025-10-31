//
// Created by 98max on 12/1/2022.
//

#include <drivers/ethernet/ethernet.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;

///__EVENT HANDLER___

EthernetDriverEventHandler::EthernetDriverEventHandler() = default;

EthernetDriverEventHandler::~EthernetDriverEventHandler() = default;

/**
 * @brief Handle data received event
 * @return True if the data should be sent back, false otherwise
 */
bool EthernetDriverEventHandler::DataReceived(uint8_t*, uint32_t) {
	return false;
}

/**
 * @brief Handle before send event
 */
void EthernetDriverEventHandler::BeforeSend(uint8_t*, uint32_t) {
}

/**
 * @brief Handle data sent event
 */
void EthernetDriverEventHandler::DataSent(uint8_t*, uint32_t) {
}

/**
 * @brief Handle an event
 *
 * @param event The event to handle
 * @return The handled event
 */
Event<EthernetDriverEvents>* EthernetDriverEventHandler::on_event(Event<EthernetDriverEvents>* event) {

	switch (event->type) {

		case EthernetDriverEvents::BEFORE_SEND:
			BeforeSend(((BeforeSendEvent*) event)->buffer, ((BeforeSendEvent*) event)->size);
			break;

		case EthernetDriverEvents::DATA_SENT:
			DataSent(((DataSentEvent*) event)->buffer, ((DataSentEvent*) event)->size);
			break;

		case EthernetDriverEvents::DATA_RECEIVED:
			event->return_value.boolValue = DataReceived(((DataReceivedEvent*) event)->buffer,
			                                             ((DataReceivedEvent*) event)->size);
			break;

		default:
			break;
	}

	return event;
}

EthernetDriver::EthernetDriver() = default;
EthernetDriver::~EthernetDriver() = default;

/**
 * @brief Get the MAC address
 *
 * @return the MAC address
 */
MediaAccessControlAddress EthernetDriver::GetMediaAccessControlAddress() {
	return 0;
}

/**
 * @brief Send data to the network via the driver backend
 *
 * @param buffer  The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::Send(uint8_t* buffer, uint32_t size) {

	// Raise the event
	raise_event(new BeforeSendEvent(buffer, size));

	DoSend(buffer, size);
}

/**
 * @brief (Device Side) Send the data
 */
void EthernetDriver::DoSend(uint8_t*, uint32_t) {
}

/**
 * @brief Handle the recieved data
 *
 * @param buffer The buffer to handle
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataReceived(uint8_t* buffer, uint32_t size) {

	// Raise the event
	Vector<Event<EthernetDriverEvents>*> values =
			raise_event(new DataReceivedEvent(buffer, size));

	// Loop through the events
	for (auto &value : values) {
		switch (value->type) {
			case EthernetDriverEvents::DATA_RECEIVED:
				if (value->return_value.boolValue)
					Send(buffer, size);
				break;

			default:
				break;
		}
	}
}

/**
 * @brief Send data
 *
 * @param buffer The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataSent(uint8_t* buffer, uint32_t size) {
	raise_event(new DataSentEvent(buffer, size));
}

// if your mac address is e.g. 1c:6f:65:07:ad:1a (see output of ifconfig)
// then you would call CreateMediaAccessControlAddress(0x1c, 0x6f, 0x65, 0x07, 0xad, 0x1a)
/**
 * @brief Create a Media Access Control Address
 *
 * @param digit1 The m_first_memory_chunk digit
 * @param digit2 The second digit
 * @param digit3 The third digit
 * @param digit4 The fourth digit
 * @param digit5 The fifth digit
 * @param digit6 The last digit
 * @return The MAC address
 */
MediaAccessControlAddress EthernetDriver::CreateMediaAccessControlAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6) {
	return // digit6 is the most significant byte
			(uint64_t) digit6 << 40
			| (uint64_t) digit5 << 32
			| (uint64_t) digit4 << 24
			| (uint64_t) digit3 << 16
			| (uint64_t) digit2 << 8
			| (uint64_t) digit1;
}

/**
 * @brief Construct a new Data Sent Event object
 *
 * @param buffer The buffer that was sent
 * @param size The size of the buffer
 */
DataSentEvent::DataSentEvent(uint8_t* buffer, uint32_t size)
		: Event(EthernetDriverEvents::DATA_SENT) {
	this->buffer = buffer;
	this->size = size;
	this->size = size;
}

DataSentEvent::~DataSentEvent() = default;

/**
 * @brief Construct a new Data Received Event object
 *
 * @param buffer The buffer that was received
 * @param size The size of the buffer
 */
DataReceivedEvent::DataReceivedEvent(uint8_t* buffer, uint32_t size)
		: Event(EthernetDriverEvents::DATA_RECEIVED) {
	this->buffer = buffer;
	this->size = size;
}

DataReceivedEvent::~DataReceivedEvent()
= default;

/**
 * @brief Construct a new Before Send Event object
 *
 * @param buffer The buffer to send
 * @param size The size of the buffer
 */
BeforeSendEvent::BeforeSendEvent(uint8_t* buffer, uint32_t size)
		: Event(EthernetDriverEvents::BEFORE_SEND) {
	this->buffer = buffer;
	this->size = size;
}

BeforeSendEvent::~BeforeSendEvent()
= default;