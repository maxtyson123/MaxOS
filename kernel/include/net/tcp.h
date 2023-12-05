//
// Created by 98max on 12/9/2022.
//

#ifndef MAXOS_NET_TCP_H
#define MAXOS_NET_TCP_H

#include <stdint.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace maxOS{

    namespace net{

        typedef uint16_t TransmissionControlProtocolPort;

        enum TransmissionControlProtocolSocketState
        {
            CLOSED,
            LISTEN,
            SYN_SENT,
            SYN_RECEIVED,

            ESTABLISHED,

            FIN_WAIT1,
            FIN_WAIT2,
            CLOSING,
            TIME_WAIT,

            CLOSE_WAIT
            //LAST_ACK
        };

        enum TransmissionControlProtocolFlag
        {
            FIN = 1,
            SYN = 2,
            RST = 4,
            PSH = 8,
            ACK = 16,
            URG = 32,
            ECE = 64,
            CWR = 128,
            NS = 256
        };


        struct TransmissionControlProtocolHeader
        {
            uint16_t srcPort;
            uint16_t dstPort;
            uint32_t sequenceNumber;
            uint32_t acknowledgementNumber;

            uint8_t reserved : 4;
            uint8_t headerSize32 : 4;
            uint8_t flags;

            uint16_t windowSize;
            uint16_t checksum;
            uint16_t urgentPtr;

            uint32_t options;
        } __attribute__((packed));


        //The pseudo header is used to calculate the checksum of the TCP header. It is a copy of the IP header, but with the protocol field set to 6 (TCP) and the length field set to the length of the TCP header.
        struct TransmissionControlProtocolPseudoHeader
        {
            uint32_t srcIP;
            uint32_t dstIP;
            uint16_t protocol;
            uint16_t totalLength;
        } __attribute__((packed));


        class TransmissionControlProtocolSocket;
        class TransmissionControlProtocolHandler;

        enum TransmissionControlProtocolPayloadHandlerEvents
        {
            CONNECTED,
            DISCONNECTED,
            DATA_RECEIVED
        };

        class DataReceivedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents>
        {
            public:
                TransmissionControlProtocolSocket* socket;
                uint8_t* data;
                uint16_t size;
                DataReceivedEvent(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size);
                ~DataReceivedEvent();
        };

        class ConnectedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents>
        {
            public:
                TransmissionControlProtocolSocket* socket;
                ConnectedEvent(TransmissionControlProtocolSocket* socket);
                ~ConnectedEvent();
        };

        class DisconnectedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents>
        {
            public:
                TransmissionControlProtocolSocket* socket;
                DisconnectedEvent(TransmissionControlProtocolSocket* socket);
                ~DisconnectedEvent();
        };

        class TransmissionControlProtocolPayloadHandler : public common::EventHandler<TransmissionControlProtocolPayloadHandlerEvents>
        {
            public:
                TransmissionControlProtocolPayloadHandler();
                ~TransmissionControlProtocolPayloadHandler();

                common::Event<TransmissionControlProtocolPayloadHandlerEvents>* onEvent(common::Event<TransmissionControlProtocolPayloadHandlerEvents>* event);

                virtual void handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size);
                virtual void Connected(TransmissionControlProtocolSocket* socket);
                virtual void Disconnected(TransmissionControlProtocolSocket* socket);
        };



        class TransmissionControlProtocolSocket : public common::EventManager<TransmissionControlProtocolPayloadHandlerEvents>
        {
            friend class TransmissionControlProtocolHandler;
            friend class TransmissionControlProtocolPortListener;
            protected:
                uint16_t remotePort;
                uint32_t remoteIP;
                uint16_t localPort;
                uint32_t localIP;
                uint32_t sequenceNumber;
                uint32_t acknowledgementNumber;

                TransmissionControlProtocolHandler* transmissionControlProtocolHandler;
                TransmissionControlProtocolSocketState state;
            public:
                TransmissionControlProtocolSocket(TransmissionControlProtocolHandler* transmissionControlProtocolHandler);
                ~TransmissionControlProtocolSocket();

                virtual void Send(uint8_t* data, uint16_t size);
                virtual void Disconnect();

                void Disconnected();
                void Connected();

                bool handleTransmissionControlProtocolPayload(uint8_t* data, uint16_t size);
        };

        class TransmissionControlProtocolHandler : InternetProtocolPayloadHandler
        {
            friend class TransmissionControlProtocolSocket;
            protected:
                common::OutputStream* errorMessages;
                common::Vector<TransmissionControlProtocolSocket*> sockets;

                static TransmissionControlProtocolPort freePorts;
                void sendTransmissionControlProtocolPacket(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size, uint16_t flags = 0);

            public:
                TransmissionControlProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
                ~TransmissionControlProtocolHandler();

                bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);

                TransmissionControlProtocolSocket* Connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port);
                TransmissionControlProtocolSocket* Connect(string internetProtocolAddressAndPort);

                void Disconnect(TransmissionControlProtocolSocket* socket);

                virtual TransmissionControlProtocolSocket* Listen(uint16_t port);
                virtual void Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolPayloadHandler* handler);
        };



    }

}

#endif //MAXOS_NET_TCP_H
