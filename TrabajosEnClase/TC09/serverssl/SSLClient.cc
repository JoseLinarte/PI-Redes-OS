/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  ****** SSLSocket example, cliente
  *
 **/

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "SSLSocket.h"

int main( int cuantos, char * argumentos[] ) {

   SSLSocket * client;
   char userName[ 16 ] = { 0 };
   char password[ 16 ] = { 0 };

   const char * requestMessage =
      "\n<Body>\n"
      "\t<UserName>%s</UserName>\n"
      "\t<Password>%s</Password>\n"
      "</Body>\n";

   char buf[ 1024 ] = { 0 };
   char clientRequest[ 1024 ] = { 0 };
   int bytes;
   char * hostname;
   int port;

   if ( cuantos != 3 ) {
      printf( "usage: %s <hostname> <portnum>\n", argumentos[ 0 ] );
      exit( 0 );
   }

   hostname = argumentos[ 1 ];
   port     = atoi( argumentos[ 2 ] );

   client = new SSLSocket();               // constructor cliente (crea contexto + SSL* propios)

   client->Connect( hostname, port );      // TCP connect + SSL_connect

   printf( "Enter the User Name : " );
   scanf( "%15s", userName );
   printf( "\nEnter the Password : " );
   scanf( "%15s", password );

   sprintf( clientRequest, requestMessage, userName, password );

   printf( "\n\nConnected with %s encryption\n", client->GetCipher() );
   client->ShowCerts();

   client->Write( clientRequest );

   bytes = client->Read( buf, sizeof( buf ) - 1 );
   buf[ bytes ] = '\0';
   printf( "Received: \"%s\"\n", buf );

   client->Close();
   delete client;

   return 0;

}