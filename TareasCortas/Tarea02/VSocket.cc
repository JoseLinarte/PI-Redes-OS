/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *******   VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
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
void VSocket::Init( char t, bool IPv6 ){
   this->IPv6 = IPv6;
   this->type = t;
   this-> port=0;
   this->sockId=-1;

   if(IPv6 == false){
      if(t == 's'){
         this->sockId = socket(AF_INET, SOCK_STREAM, 0);
      }else if(t == 'd'){
         this->sockId = socket(AF_INET, SOCK_DGRAM,0);
      }
   }else{
      if(t == 's'){
         this->sockId = socket(AF_INET6, SOCK_STREAM, 0);
      }else if(t == 'd'){
         this->sockId = socket(AF_INET6, SOCK_DGRAM,0);
      }
   }

   if ( -1 == this->sockId ) {
      throw std::runtime_error( "VSocket::Init, (reason)" );
   }
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
   int st = 0;

   if(this->sockId>=0){
      st=close(this->sockId);
      this->sockId=-1;
   }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
   }

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st = -1;
   this->port=port;

   if(this->IPv6 == false){   
      struct sockaddr_in host4;
      memset((char*)&host4, 0, sizeof(host4));
      host4.sin_family = AF_INET;
      st = inet_pton(AF_INET, hostip, &host4.sin_addr);
      if(-1 == st){
         throw std::runtime_error("VSocket::TryToConnect : pton");
      }
      host4.sin_port = htons(port);
      st = connect(sockId, (sockaddr *)&host4, sizeof(host4));
      if(-1 == st){
         throw std::runtime_error("Vsocket::trytoconnect : htons");
      }
   } else{
      struct sockaddr_in6  host6;
      struct sockaddr * ha;

      memset( &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      st = inet_pton( AF_INET6, hostip, &host6.sin6_addr );
      if (st <= 0 ) {
         throw std::runtime_error( "Socket::Connect( const char *, int ) [inet_pton]" );
      }
      host6.sin6_port = htons( port );
      ha = (struct sockaddr *) &host6;
      int len = sizeof( host6 );
      st = connect( sockId, ha, len );
      if ( -1 == st ) {
         throw std::runtime_error( "VSocket::TryToConnect( const char *, int ) [connect]" );
      }
   }  
   return st;

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect( const char *host, const char *service ) {
   int st;
   struct addrinfo hints, *result, *rp;

   memset(&hints, 0, sizeof(struct addrinfo));
   if (this->IPv6) {
      hints.ai_family = AF_INET6;
   } else {
      hints.ai_family = AF_INET;
   }

   if('s'==this->type){
      hints.ai_socktype= SOCK_STREAM;
   }else{
      hints.ai_socktype= SOCK_DGRAM;
   }

   st=getaddrinfo(host,service,&hints,&result);
   if(0!=st){
      throw std::runtime_error( "VSocket::TryToConnect, getaddrinfo" );
   }

   for(rp=result; nullptr!=rp; rp= rp->ai_next){
      st= connect(this-> sockId, rp->ai_addr, rp->ai_addrlen);
      if(0==st){ 
         break;
      }
   }
   freeaddrinfo(result); 

   if(-1==st){
      throw std::runtime_error( "VSocket::TryToConnect,connect" );
   }

   return st;


}


/**
  * Bind method
  *  
  *   solo IPv4 por ahora
  *
  * @param      int port: puerto local donde el servidor va a escuchar
  *
 **/
int VSocket::Bind( int port ) {

   int st;
   this->port = port;

   if ( this->IPv6 == false ) {
      struct sockaddr_in host4;
      memset( &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      host4.sin_addr.s_addr = htonl( INADDR_ANY );
      host4.sin_port = htons( port );

      st = bind( sockId, (struct sockaddr *) &host4, sizeof( host4 ) );
   } else {
      struct sockaddr_in6 host6;
      memset( &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      host6.sin6_addr = in6addr_any;
      host6.sin6_port = htons( port );

      st = bind( sockId, (struct sockaddr *) &host6, sizeof( host6 ) );
   }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Bind" );
   }

   return st;
}


/**
  * sendTo method
  * 
  *
  * @param      void * buffer: datos a enviar
  * @param      size_t size: tamaño del buffer
  * @param      void * dest: struct sockaddr_in * del destinatario
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * dest ) {

   ssize_t st;
   socklen_t addrlen = this->IPv6 ? sizeof( struct sockaddr_in6 ): sizeof( struct sockaddr_in );

   st = sendto( sockId, buffer, size, 0, (struct sockaddr *) dest, addrlen );
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::sendTo" );
   }

   return (size_t) st;
}


/**
  * recvFrom method
  *
  * @param      void * buffer: buffer donde se guardan los datos recibidos
  * @param      size_t size: capacidad del buffer
  * @param      void * from: struct sockaddr_in * donde se guarda la dirección del emisor
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * from ) {

   ssize_t st;
   socklen_t len = this->IPv6 ? sizeof( struct sockaddr_in6 ) : sizeof( struct sockaddr_in );

   st = recvfrom( sockId, buffer, size, 0, (struct sockaddr *) from, &len );
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::recvFrom" );
   }

   return (size_t) st;
}
