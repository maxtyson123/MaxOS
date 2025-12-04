/**
 * @file ethernet.h
 * @brief Defines an EthernetDriver class for managing Ethernet communication, including sending and receiving data, handling MAC addresses, and event management.
 *
 * @date 1st December 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_ETHERNET_H
#define MAXOS_DRIVERS_ETHERNET_ETHERNET_H

#include <cstdint>
#include <drivers/driver.h>
#include <vector.h>
#include <eventHandler.h>


namespace MaxOS::drivers::ethernet {

	/// Used to make MAC addresses more readable @todo make a MacAddress class and use mac_t
	typedef uint64_t MediaAccessControlAddress;

	/**
	 * @enum EthernetDriverEvents
	 * @brief Events that can be triggered by the Ethernet Driver
	 */
	enum class EthernetDriverEvents {
		BEFORE_SEND,
		DATA_SENT,
		DATA_RECEIVED
	};

	/**
	 * @class BeforeSendEvent
	 * @brief Event that is triggered before data is sent, holds the buffer and size of the data
	 */
	class BeforeSendEvent : public common::Event<EthernetDriverEvents> {
		public:
			uint8_t* buffer;    ///< The buffer to be sent
			uint32_t size;      ///< The size of the buffer to be sent
			BeforeSendEvent(uint8_t* buffer, uint32_t size);
			~BeforeSendEvent();
	};

	/**
	 * @class DataSentEvent
	 * @brief Event that is triggered when data is sent, holds the buffer and size of the data
	 */
	class DataSentEvent : public common::Event<EthernetDriverEvents> {
		public:
			uint8_t* buffer;    ///< The buffer that was sent
			uint32_t size;      ///< The size of the buffer that was sent
			DataSentEvent(uint8_t* buffer, uint32_t size);
			~DataSentEvent();
	};

	/**
	 * @class DataReceivedEvent
	 * @brief Event that is triggered when data is received, holds the buffer and size of the data
	 */
	class DataReceivedEvent : public common::Event<EthernetDriverEvents> {
		public:
			uint8_t* buffer;     ///< @deprecated WRS
			uint32_t size;       ///< @deprecated WRS
			DataReceivedEvent(uint8_t* buffer, uint32_t size);
			~DataReceivedEvent();
	};

	/**
	 * @class EthernetDriverEventHandler
	 * @brief Handles the events that are triggered by the Ethernet Driver
	 */
	class EthernetDriverEventHandler : public common::EventHandler<EthernetDriverEvents> {
		public:
			EthernetDriverEventHandler();
			~EthernetDriverEventHandler();

			common::Event<EthernetDriverEvents>* on_event(common::Event<EthernetDriverEvents>* event) override;

			virtual void before_send(uint8_t* buffer, uint32_t size);
			virtual void data_sent(uint8_t* buffer, uint32_t size);
			virtual bool data_received(uint8_t* buffer, uint32_t size);
	};

	/**
	 * @class EthernetDriver
	 * @brief Driver for the Ethernet Controller, manages the sending and receiving of data, the mac address, and the events
	 */
	class EthernetDriver : public Driver, public common::EventManager<EthernetDriverEvents> {
		protected:
			virtual void do_send(uint8_t* buffer, uint32_t size);
			void fire_data_received(uint8_t* buffer, uint32_t size);
			void fire_data_sent(uint8_t* buffer, uint32_t size);

		public:
			EthernetDriver();
			~EthernetDriver();

			static MediaAccessControlAddress create_media_access_control_address(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6);
			virtual MediaAccessControlAddress get_media_access_control_address();

			void send(uint8_t* buffer, uint32_t size);
	};

}


#endif //MAXOS_DRIVERS_ETHERNET_ETHERNET_H
