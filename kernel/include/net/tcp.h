//
// Created by 98max on 12/9/2022.
//

#ifndef MAXOS_NET_TCP_H
#define MAXOS_NET_TCP_H

#include <common/types.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace maxOS{

    namespace net{

        typedef common::uint16_t TransmissionControlProtocolPort;

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
            common::uint16_t srcPort;
            common::uint16_t dstPort;
            common::uint32_t sequenceNumber;
            common::uint32_t acknowledgementNumber;

            common::uint8_t reserved : 4;
            common::uint8_t headerSize32 : 4;
            common::uint8_t flags;

            common::uint16_t windowSize;
            common::uint16_t checksum;
            common::uint16_t urgentPtr;

            common::uint32_t options;
        } __attribute__((packed));


        //The pseudo header is used to calculate the checksum of the TCP header. It is a copy of the IP header, but with the protocol field set to 6 (TCP) and the length field set to the length of the TCP header.
        struct TransmissionControlProtocolPseudoHeader
        {
            common::uint32_t srcIP;
            common::uint32_t dstIP;
            common::uint16_t protocol;
            common::uint16_t totalLength;
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
                common::uint8_t* data;
                common::uint16_t size;
                DataReceivedEvent(TransmissionControlProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
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

                virtual void handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
                virtual void Connected(TransmissionControlProtocolSocket* socket);
                virtual void Disconnected(TransmissionControlProtocolSocket* socket);
        };



        class TransmissionControlProtocolSocket : public common::EventManager<TransmissionControlProtocolPayloadHandlerEvents>
        {
            friend class TransmissionControlProtocolHandler;
            friend class TransmissionControlProtocolPortListener;
            protected:
                common::uint16_t remotePort;
                common::uint32_t remoteIP;
                common::uint16_t localPort;
                common::uint32_t localIP;
                common::uint32_t sequenceNumber;
                common::uint32_t acknowledgementNumber;

                TransmissionControlProtocolHandler* transmissionControlProtocolHandler;
                TransmissionControlProtocolSocketState state;
            public:
                TransmissionControlProtocolSocket(TransmissionControlProtocolHandler* transmissionControlProtocolHandler);
                ~TransmissionControlProtocolSocket();

                virtual void Send(common::uint8_t* data, common::uint16_t size);
                virtual void Disconnect();

                void Disconnected();
                void Connected();

                bool handleTransmissionControlProtocolPayload(common::uint8_t* data, common::uint16_t size);
        };

        class TransmissionControlProtocolHandler : InternetProtocolPayloadHandler
        {
            friend class TransmissionControlProtocolSocket;
            protected:
                common::OutputStream* errorMessages;
                common::Vector<TransmissionControlProtocolSocket*> sockets;

                static TransmissionControlProtocolPort freePorts;
                void sendTransmissionControlProtocolPacket(TransmissionControlProtocolSocket* socket, common::uint8_t* data, common::uint16_t size, common::uint16_t flags = 0);

            public:
                TransmissionControlProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
                ~TransmissionControlProtocolHandler();

                bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, common::uint8_t* payloadData, common::uint32_t size);

                TransmissionControlProtocolSocket* Connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port);
                TransmissionControlProtocolSocket* Connect(common::string internetProtocolAddressAndPort);

                void Disconnect(TransmissionControlProtocolSocket* socket);

                virtual TransmissionControlProtocolSocket* Listen(common::uint16_t port);
                virtual void Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolPayloadHandler* handler);
        };



    }

}

#endif //MAXOS_NET_TCP_H
