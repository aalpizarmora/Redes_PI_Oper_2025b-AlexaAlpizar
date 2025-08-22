/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>			// getaddrinfo, freeaddrinfo
#include <unistd.h>			// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::BuildSocket( char t, bool IPv6 ){

   int st = -1;
   int domain = AF_INET;
   int type = SOCK_STREAM;
   int protocol = 0;

   if (t =='s') {
      type = SOCK_STREAM;
      this->type = SOCK_STREAM;
   }
   else if (t =='d'){ 
    type = SOCK_DGRAM;
    this->type = SOCK_DGRAM;
   }

   if (IPv6 == true) {
    domain = AF_INET6;
    this->IPv6 = true;
    this->domain = AF_INET6;
   }
   else { 
    domain = AF_INET;
    this->IPv6 = false;
    this->domain = AF_INET;
   }


   st = socket(domain,type,protocol);

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::BuildSocket, (reason)" );
   }

   this->idSocket = st;

}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {

   this->Close();

}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){
   int st = -1;
   st = close(this->idSocket);
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
   }

}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::EstablishConnection( const char * hostip, int port ) {

   // Como utilizar la estructura de conexión
   // Para IPv4
    int st;
    struct sockaddr_in  host4;
    memset( (char *) &host4, 0, sizeof( host4 ) );

    host4.sin_family = this->domain;
    st = inet_pton( this->domain, hostip, &host4.sin_addr );
    if ( -1 == st ) {
        throw( std::runtime_error( "VSocket::DoConnect, inet_pton" ));
    }
    host4.sin_port = htons( port );
    st = connect( idSocket, (sockaddr *) &host4, sizeof( host4 ) );
    if ( -1 == st ) {
        throw( std::runtime_error( "VSocket::DoConnect, connect" ));
    }
    
    return st;

}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::EstablishConnection( const char *host, const char *service ) {
   int st = -1;
    struct addrinfo hints, *res, *p;

    // Limpiar hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = IPv6 ? AF_INET6 : AF_INET; // usar IPv6 o IPv4
    hints.ai_socktype = (type == 's') ? SOCK_STREAM : SOCK_DGRAM; // stream (TCP) o datagram (UDP)

    // Resolver dirección
    int status = getaddrinfo(host, service, &hints, &res);
    if (status != 0) {
        throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(status));
    }

    // Recorrer las posibles direcciones y probar conexión
    for (p = res; p != nullptr; p = p->ai_next) {
        // Crear socket
        idSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (idSocket == -1) {
            continue; // error, probar siguiente
        }

        // Intentar conectar
        if (connect(idSocket, p->ai_addr, p->ai_addrlen) == -1) {
            close(idSocket);
            idSocket = -1;
            continue; // probar siguiente dirección
        }

        // Conexión exitosa
        st = 0;
        break;
    }

    freeaddrinfo(res); // liberar memoria

    if (st == -1) {
        throw std::runtime_error("VSocket::EstablishConnection - unable to connect");
    }

    return st;

}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {
   int st = -1;

      struct sockaddr_in host4;

      host4.sin_family = AF_INET;
      host4.sin_addr.s_addr = htonl( INADDR_ANY );
      host4.sin_port = htons( port );
      memset(host4.sin_zero, '\0', sizeof (host4.sin_zero));
      st = bind(this->idSocket, (struct sockaddr *) &host4, sizeof(host4));

   return st;

}


/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   int st = -1;
   socklen_t addrSocklen;
   addrSocklen = sizeof (struct sockaddr_in);
   st = sendto(this->idSocket, buffer, size, 0,(struct sockaddr *) addr, addrSocklen);
   return st;

}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   int st = -1;
   socklen_t addrSocklen;
   addrSocklen = sizeof (struct sockaddr_in);
   st = recvfrom(this->idSocket, buffer, size, 0,(struct sockaddr *) addr, &addrSocklen);
   return st;

}

