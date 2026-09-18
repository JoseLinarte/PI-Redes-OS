/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  ****** SSLSocket example, server con fork
  *
 **/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

#include "SSLSocket.h"

#define PORT 4321


int main( int cuantos, char ** argumentos ) {

   SSLSocket * server;
   VSocket   * clienteBase;
   int port = PORT;
   int childpid;

   if ( cuantos > 1 ) {
      port = atoi( argumentos[ 1 ] );
   }

   bool useIPv6 = ( cuantos > 2 && 0 == strcmp( argumentos[ 2 ], "-6" ) );
   server = new SSLSocket( "ci0123.pem", "key0123.pem", useIPv6 );
   server->Bind( port );
   server->MarkPassive( 10 );

   printf( "[SSL SERVER - FORK] Escuchando en el puerto %d\n", port );

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

   for ( ; ; ) {

      clienteBase = server->AcceptConnection();
      SSLSocket * client = (SSLSocket *) clienteBase;
      client->CopyContext( server );

      childpid = fork();

      if ( childpid < 0 ) {
         perror( "server: fork error" );
      } else if ( 0 == childpid ) {              // codigo del hijo

         char buf[ 1024 ] = { 0 };
         int bytes;

         client->Accept();
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
         exit( 0 );

      }

      delete client;          // el padre no necesita este objeto, ya lo maneja el hijo
      while ( waitpid( -1, nullptr, WNOHANG ) > 0 ) { }   // limpia procesos hijos terminados, sin bloquear

   }

}