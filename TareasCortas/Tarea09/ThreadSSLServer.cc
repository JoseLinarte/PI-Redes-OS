/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  ****** SSLSocket example, server con hilos
  *
 **/

#include <thread>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "SSLSocket.h"

#define PORT 4321


void Service( VSocket * clienteBase ) {

   SSLSocket * client = (SSLSocket *) clienteBase;

   char buf[ 1024 ] = { 0 };
   int bytes;

   const char * ServerResponse =
      "\n<Body>\n"
      "\t<Server>os.ecci.ucr.ac.cr</Server>\n"
      "\t<dir>ci0123</dir>\n"
      "\t<Name>Proyecto Integrador Redes y sistemas Operativos</Name>\n"
      "\t<NickName>PIRO</NickName>\n"
      "\t<Description>Consolidar e integrar los conocimientos de redes y sistemas operativos</Description>\n"
      "\t<Author>profesores PIRO</Author>\n"
      "</Body>\n";

   const char * validMessage =
      "\n<Body>\n"
      "\t<UserName>piro</UserName>\n"
      "\t<Password>ci0123</Password>\n"
      "</Body>\n";

   client->Accept();          // handshake SSL
   client->ShowCerts();

   bytes = client->Read( buf, sizeof( buf ) - 1 );
   buf[ bytes ] = '\0';
   printf( "Client msg: \"%s\"\n", buf );

   if ( 0 == strcmp( validMessage, buf ) ) {
      client->Write( ServerResponse );
   } else {
      client->Write( "Invalid Message" );
   }

   client->Close();
   delete client;

}


int main( int cuantos, char ** argumentos ) {

   SSLSocket * server;
   VSocket   * clienteBase;
   std::thread * worker;
   int port = PORT;

   if ( cuantos > 1 ) {
      port = atoi( argumentos[ 1 ] );
   }
   bool useIPv6 = ( cuantos > 2 && 0 == strcmp( argumentos[ 2 ], "-6" ) );
   server = new SSLSocket( "ci0123.pem", "key0123.pem", useIPv6 );
   server->Bind( port );
   server->MarkPassive( 10 );

   printf( "[SSL SERVER - HILOS] Escuchando en el puerto %d\n", port );

   for ( ; ; ) {
      clienteBase = server->AcceptConnection();
      ( (SSLSocket *) clienteBase )->CopyContext( server );
      worker = new std::thread( Service, clienteBase );
      worker->detach();
   }

}