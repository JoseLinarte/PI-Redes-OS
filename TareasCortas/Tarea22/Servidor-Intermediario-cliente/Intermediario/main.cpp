#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "ServicioProductosDistribuido.hpp"
#include "Servidor.hpp"
#include "Descubrimiento.hpp"
#include "DescubrimientoIslas.hpp"
#include "ServidorIslas.hpp"
#include "Broadcasts.hpp"

static std::atomic<bool> salir( false );
static void ManejarSenal( int ) { salir = true; }

int main( int argc, char * argv[] ) {
   signal( SIGPIPE, SIG_IGN );

   const int PUERTO_ISLAS = 9090;
   int puerto = ( argc > 1 ) ? std::atoi( argv[1] ) : 8080;
   std::string broadcast;
   int isla = 3;
   std::string miIp;
   std::string destinos;
   std::string publica;

   ServicioProductosDistribuido servicio;
   for ( int i = 2; i < argc; i++ ) {
      std::string arg = argv[i];
      if ( arg.rfind( "isla=", 0 ) == 0 ) { isla = std::atoi( arg.c_str() + 5 ); continue; }
      if ( arg.rfind( "ip=", 0 ) == 0 ) { miIp = arg.substr( 3 ); continue; }
      if ( arg.rfind( "destinos=", 0 ) == 0 ) { destinos = arg.substr( 9 ); continue; }
      if ( arg.rfind( "publica=", 0 ) == 0 ) { publica = arg.substr( 8 ); continue; }

      size_t dp = arg.rfind( ':' );
      if ( dp == std::string::npos ) {
         broadcast = arg;
         continue;
      }
      int p = std::atoi( arg.substr( dp + 1 ).c_str() );
      if ( p <= 0 ) {
         std::cerr << "Argumento invalido (use host:puerto): " << arg << "\n";
         return 1;
      }
      servicio.AgregarSemilla( arg.substr( 0, dp ), p );
   }
   if ( isla < 1 || isla > 6 ) {
      std::cerr << "isla debe estar entre 1 y 6\n";
      return 1;
   }

   std::string ipPrivada = MiIpPrivada();
   if ( publica.empty() ) publica = MiIpPublica();
   if ( publica == "todas" ) publica = "";

   Servidor servidor( puerto, publica );
   if ( !servidor.Iniciar() ) {
      return 1;
   }
   servicio.IniciarMonitor( 10 );
   servicio.IniciarHilosIslas( isla, 10 );

   Descubrimiento descubrimiento( servicio, puerto, ResolverBroadcasts( broadcast ) );
   descubrimiento.Iniciar( 10 );

   ServidorIslas servidorIslas( servicio, PUERTO_ISLAS, ipPrivada );
   servidorIslas.Iniciar();

   char idIsla[16];
   std::snprintf( idIsla, sizeof( idIsla ), "INT_%02d", isla );
   if ( miIp.empty() ) miIp = ipPrivada;
   std::vector<std::string> dest = destinos.empty() ? BroadcastsOtrasIslas( isla ) : ResolverBroadcasts( destinos );
   DescubrimientoIslas islas( servicio, idIsla, miIp, PUERTO_ISLAS, dest );
   islas.Iniciar( 10 );

   std::signal( SIGINT, ManejarSenal );
   std::signal( SIGTERM, ManejarSenal );
   std::thread( [&islas]() {
      while ( !salir ) std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
      islas.Detener();
      std::_Exit( 0 );
   } ).detach();

   servidor.Ejecutar( servicio );
   islas.Detener();
   return 0;
}