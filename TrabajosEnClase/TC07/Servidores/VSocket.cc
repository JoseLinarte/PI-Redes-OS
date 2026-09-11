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
      if ( st <= 0 ) {
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

   bool connected = false;
   for(rp=result; nullptr!=rp; rp= rp->ai_next){
      st= connect(this-> sockId, rp->ai_addr, rp->ai_addrlen);
      if(0==st){
         connected = true;
         break;
      }
   }
   freeaddrinfo(result); 

   if(!connected){
      throw std::runtime_error( "VSocket::TryToConnect,connect" );
   }

   return st;


}



/**
  * BuildSocket method
  *   construye una instancia usando un descriptor ya existente
  *   (por ejemplo, el que devuelve "accept")
  *
  * @param      int id: descriptor de socket ya creado
  *
 **/
void VSocket::BuildSocket( int id ) {
   this->sockId = id;
   this->IPv6   = false;
   this->port   = 0;
   this->type   = 's';
}


/**
  * MarkPassive method
  *   usa el llamado al sistema "listen" para marcar el socket
  *   como pasivo (receptor de conexiones)
  *
  * @param      int backlog: tamaño de la cola de conexiones pendientes
  *
 **/
int VSocket::MarkPassive( int backlog ) {
   int st;

   st = listen( this->sockId, backlog );
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::MarkPassive, listen" );
   }
   return st;
}


/**
  * Bind method
  *   asocia el socket con un puerto local (IPv4)
  *
  * @param      int port: número de puerto a utilizar
  *
 **/
int VSocket::Bind( int port ) {
   int st;
   struct sockaddr_in server_addr;

   this->port = port;
   memset( &server_addr, 0, sizeof( server_addr ) );
   server_addr.sin_family      = AF_INET;
   server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
   server_addr.sin_port        = htons( port );

   st = bind( this->sockId, (const sockaddr *) &server_addr, sizeof( server_addr ) );
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Bind, bind" );
   }
   return st;
}


/**
  * Shutdown method
  *   cierra parcialmente el socket según el parámetro dado
  *   (SHUT_RD, SHUT_WR, SHUT_RDWR)
  *
  * @param      int how: modo de cierre parcial
  *
 **/
int VSocket::Shutdown( int how ) {
   int st;

   st = shutdown( this->sockId, how );
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Shutdown, shutdown" );
   }
   return st;
}

