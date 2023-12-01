//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <common/types.h>
#include <common/eventHandler.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace maxOS
{
    namespace net
    {

        struct UserDatagramProtocolHeader
        {

            common::uint16_t sourcePort;
            common::uint16_t destinationPort;
            common::uint16_t length;
            common::uint16_t checksum;

        } __attribute__((packed));

        enum UserDatagramProtocolEvents{
            UDP_DATA_RECEIVED,
        };

        //Predefine
        class UserDatagramProtocolSocket;
        class UserDatagramProtocolHandler;

        typedef common::uint16_t UserDatagramProtocolPort;

        class UDPDataReceivedEvent : public common::Event<UserDatagramProtocolEvents>{
            public:
                UserDatagramProtocolSocket* socket;
                common::uint8_t* data;
                common::uint16_t size;

                UDPDataReceivedEvent(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
                ~UDPDataReceivedEvent();
        };

        class UserDatagramProtocolPayloadHandler: public common::EventHandler<UserDatagramProtocolEvents>
        {
            public:
                UserDatagramProtocolPayloadHandler();
                ~UserDatagramProtocolPayloadHandler();

                common::Event<UserDatagramProtocolEvents>* onEvent(common::Event<UserDatagramProtocolEvents>* event);

                virtual void handleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);

        };

        class UserDatagramProtocolSocket : public common::EventManager<UserDatagramProtocolEvents>
        {
            friend class UserDatagramProtocolHandler;
            protected:
                bool listening;

                common::uint16_t localPort;
                common::uint16_t remotePort;

                common::uint32_t localIP;
                common::uint32_t remoteIP;

                UserDatagramProtocolHandler* userDatagramProtocolHandler;

            public:
                UserDatagramProtocolSocket();
                ~UserDatagramProtocolSocket();  

                virtual void handleUserDatagramProtocolPayload(common::uint8_t* data, common::uint16_t size);
                virtual void Send(common::uint8_t* data, common::uint16_t size);
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

                virtual bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, common::uint8_t* payloadData, common::uint32_t size);
                
                UserDatagramProtocolSocket* Connect(common::uint32_t ip, common::uint16_t port);
                UserDatagramProtocolSocket* Connect(common::string internetProtocolAddressAndPort);

                UserDatagramProtocolSocket* Listen(common::uint16_t port);

                void Disconnect(UserDatagramProtocolSocket* socket);
                void Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);

                void Bind(UserDatagramProtocolSocket *socket, UserDatagramProtocolPayloadHandler *userDatagramProtocolPayloadHandler);
        };

    }
}

#endif //MAXOS_NET_UDP_H
