/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *
  *  Ejemplo de sockets con IPv4
  *
 **/

#include <stdio.h>
#include <string.h>

#include "VSocket.h"
#include "Socket.h"

int main( int argc, char * argv[] ) {
   const char * os = "http://os.ecci.ucr.ac.cr/";
   const char * osi = "10.84.166.62";
   const char * ose = "163.178.104.62";
   const char * alimentos = (char *) "GET /TicAmazon/list.php?category=Alimentos%20y%20bebidas HTTP/1.1\r\nhost: redes.ecci\r\n\r\n";

   VSocket * s;	
   char a[512];


   s = new Socket( 's' );
   s->Connect( ose, 80 );	// usar "osi" en la ECCI, "ose" de sus casas
   s->Write( alimentos );

   int leido;
   do{
      memset(a,0, 512);
      leido = s->Read(a,512);
      if(leido > 0){
         printf("%s\n", a);
      }

   }while(leido > 0);

}

