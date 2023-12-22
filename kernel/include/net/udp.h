//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <stdint.h>
#include <common/eventHandler.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace maxOS
{
    namespace net
    {

        struct UserDatagramProtocolHeader
        {

            uint16_t sourcePort;
            uint16_t destinationPort;
            uint16_t length;
            uint16_t checksum;

        } __attribute__((packed));

        enum UserDatagramProtocolEvents{
            UDP_DATA_RECEIVED,
        };

        //Predefine
        class UserDatagramProtocolSocket;
        class UserDatagramProtocolHandler;

        typedef uint16_t UserDatagramProtocolPort;

        class UDPDataReceivedEvent : public common::Event<UserDatagramProtocolEvents>{
            public:
                UserDatagramProtocolSocket* socket;
                uint8_t* data;
                uint16_t size;

                UDPDataReceivedEvent(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);
                ~UDPDataReceivedEvent();
        };

        class UserDatagramProtocolPayloadHandler: public common::EventHandler<UserDatagramProtocolEvents>
        {
            public:
                UserDatagramProtocolPayloadHandler();
                ~UserDatagramProtocolPayloadHandler();

                common::Event<UserDatagramProtocolEvents>*
                on_event(common::Event<UserDatagramProtocolEvents>* event);

                virtual void handleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);

        };

        class UserDatagramProtocolSocket : public common::EventManager<UserDatagramProtocolEvents>
        {
            friend class UserDatagramProtocolHandler;
            protected:
                bool listening;

                uint16_t localPort;
                uint16_t remotePort;

                uint32_t localIP;
                uint32_t remoteIP;

                UserDatagramProtocolHandler* userDatagramProtocolHandler;

            public:
                UserDatagramProtocolSocket();
                ~UserDatagramProtocolSocket();  

                virtual void handleUserDatagramProtocolPayload(uint8_t* data, uint16_t size);
                virtual void Send(uint8_t* data, uint16_t size);
                virtual void Disconnect();

        }; 

        class UserDatagramProtocolHandler : InternetProtocolPayloadHandler
        {
            protected:
                common::Vector<UserDatagramProtocolSocket*> sockets;
                static UserDatagramProtocolPort freePorts;
                common::OutputStream* errorMessages;

            public:
                UserDatagramProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
                ~UserDatagramProtocolHandler();

                virtual bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
                
                UserDatagramProtocolSocket* Connect(uint32_t ip, uint16_t port);
                UserDatagramProtocolSocket* Connect(string internetProtocolAddressAndPort);

                UserDatagramProtocolSocket* Listen(uint16_t port);

                void Disconnect(UserDatagramProtocolSocket* socket);
                void Send(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);

                void Bind(UserDatagramProtocolSocket *socket, UserDatagramProtocolPayloadHandler *userDatagramProtocolPayloadHandler);
        };

    }
}

#endif //MAXOS_NET_UDP_H
