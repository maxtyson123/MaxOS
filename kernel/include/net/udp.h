//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <common/types.h>
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

        //Predefine
        class UserDatagramProtocolSocket;
        class UserDatagramProtocolProvider;


        class UserDatagramProtocolHandler
        {
            public:
                UserDatagramProtocolHandler();
                ~UserDatagramProtocolHandler();
                virtual void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);

        };

        class UserDatagramProtocolSocket
        {
            friend class UserDatagramProtocolProvider;
            protected:
                bool listening;

                common::uint16_t localPort;
                common::uint16_t remotePort;

                common::uint32_t localIP;
                common::uint32_t remoteIP;
                
                UserDatagramProtocolProvider* backend;
                UserDatagramProtocolHandler* handler;

            public:
                UserDatagramProtocolSocket(UserDatagramProtocolProvider* backend);
                ~UserDatagramProtocolSocket();  

                virtual void HandleUserDatagramProtocolMessage(common::uint8_t* data, common::uint16_t size);
                virtual void Send(common::uint8_t* data, common::uint16_t size);
                virtual void Disconnect();

        }; 

        class UserDatagramProtocolProvider : InternetProtocolHandler
        {
            protected:
                UserDatagramProtocolSocket* sockets[65536];
                common::uint16_t numSockets;
                common::uint16_t freePort;

            public:
                UserDatagramProtocolProvider(InternetProtocolProvider* backend);
                ~UserDatagramProtocolProvider();

                virtual bool OnInternetProtocolReceived(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internetprotocolPayload, common::uint32_t size);
                
                virtual UserDatagramProtocolSocket* Connect(common::uint32_t ip, common::uint16_t port);
                virtual UserDatagramProtocolSocket* Listen(common::uint16_t port);
                
                virtual void Disconnect(UserDatagramProtocolSocket* socket);
                virtual void Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);

                virtual void Bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolHandler* handler);
        };

    }
}

#endif //MAXOS_NET_UDP_H
