#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include "Socket.hpp"
#include "ManejadorIntermediario.hpp"

int main( int argc, char * argv[] ) {
   int puerto = ( argc > 1 ) ? std::atoi( argv[1] ) : 8081;
   std::string hostProductos = ( argc > 2 ) ? argv[2] : "127.0.0.1";
   std::string puertoProductos = ( argc > 3 ) ? argv[3] : "8080";
   std::signal( SIGPIPE, SIG_IGN );
   Socket escucha( 's', false );
   try {
      escucha.Bind( puerto );
      escucha.Listen( 16 );
   } catch ( const std::exception & e ) {
      std::cerr << "No se pudo iniciar el intermediario: " << e.what() << "\n";
      return 1;
   }

   std::cout << "Intermediario escuchando en el puerto " << puerto
             << " -> productos en " << hostProductos << ":" << puertoProductos << "\n";

   while ( true ) {
      try {
         Socket cliente = escucha.Accept();
         std::thread hilo( &ManejadorIntermediario::Atender, std::move( cliente ),hostProductos, puertoProductos );
         hilo.detach();
      } catch ( const std::exception & e ) {
         std::cerr << "[Intermediario] Accept() fallo: " << e.what() << "\n";
      }
   }
   return 0;
}