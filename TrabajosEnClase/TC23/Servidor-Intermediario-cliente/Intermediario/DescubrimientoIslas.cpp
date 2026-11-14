#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include "DescubrimientoIslas.hpp"
#include "ProtocoloIslas.hpp"
#include "Bitacora.hpp"

static bool Digitos( const std::string & s, size_t min, size_t max ) {
   if ( s.size() < min || s.size() > max ) return false;
   for ( unsigned char c : s ) {
      if ( !std::isdigit( c ) ) return false;
   }
   return true;
}

static bool IdIslaValido( const std::string & s ) {
   return s.size() == 6 && s.compare( 0, 4, "INT_" ) == 0 && Digitos( s.substr( 4 ), 2, 2 );
}

static bool IpValida( const std::string & s ) {
   std::vector<std::string> o = ProtocoloIslas::Partir( s, '.' );
   if ( o.size() != 4 ) return false;
   for ( const std::string & x : o ) {
      if ( !Digitos( x, 1, 3 ) || std::stoi( x ) > 255 ) return false;
   }
   return true;
}

DescubrimientoIslas::DescubrimientoIslas( ServicioProductosDistribuido & servicio, const std::string & miId,const std::string & miIp, int puerto,const std::vector<std::string> & destinos ): servicio( servicio ), miId( miId ), miIp( miIp ), puerto( puerto ), destinos( destinos ),udp( 'd', false ), corriendo( false ),contador( (int)( std::time( nullptr ) % 9999 ) ) {}   // arranque distinto en cada corrida

DescubrimientoIslas::~DescubrimientoIslas() {
   Detener();
}

bool DescubrimientoIslas::Iniciar( int segundos ) {
   try {
      udp.HabilitarBroadcast();
      udp.Bind( puerto );
      udp.SetReadTimeout( 1 );   // para poder revisar 'corriendo'
   } catch ( const std::exception & e ) {
      Bitacora::Registrar( "ERROR", std::string( "No se pudo iniciar el descubrimiento entre islas (UDP "+ std::to_string( puerto ) + "): " ) + e.what(), "intermediario" );
      return false;
   }

   corriendo = true;
   hiloEscucha = std::thread( [this]() { HiloEscucha(); } );
   hiloAnuncio = std::thread( [this, segundos]() { HiloAnuncio( segundos ); } );

   std::string lista;
   for ( const std::string & d : destinos ) lista += ( lista.empty() ? "" : ", " ) + d;
   if ( miIp.empty() ) {
      Bitacora::Registrar( "INFO", "Descubrimiento entre islas: no hay IP 172.16.123.x, solo se escucha (UDP "+ std::to_string( puerto ) + ")", "intermediario" );
   } else {
      Bitacora::Registrar( "INFO", "Descubrimiento entre islas activo: " + miId + " en " + miIp + ":" + std::to_string( puerto ) + ", anuncia a " + lista, "intermediario" );
   }
   return true;
}

void DescubrimientoIslas::Detener() {
   if ( !corriendo ) return;
   corriendo = false;
   if ( hiloAnuncio.joinable() ) hiloAnuncio.join();
   if ( hiloEscucha.joinable() ) hiloEscucha.join();

   if ( !miIp.empty() ) {
      for ( int i = 0; i < 2; i++ ) {
         for ( const std::string & d : destinos ) {
            udp.SendTo( "61/" + miId, d, puerto );
         }
      }
      Bitacora::Registrar( "INFO", "Se anuncio la salida de " + miId + " a las otras islas", "intermediario" );
   }
}

void DescubrimientoIslas::Anunciar() {
   if ( miIp.empty() ) return;
   contador = ( contador % 9999 ) + 1;
   char idMsg[8];
   std::snprintf( idMsg, sizeof( idMsg ), "%04d", contador );
   std::string msg = std::string( "60/" ) + idMsg + "/" + miId + "/" + miIp + "/" + std::to_string( puerto );
   for ( const std::string & d : destinos ) {
      udp.SendTo( msg, d, puerto );
   }
}

void DescubrimientoIslas::HiloAnuncio( int segundos ) {
   while ( corriendo ) {
      Anunciar();
      for ( int i = 0; i < segundos && corriendo; i++ ) {
         std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
      }
   }
}

bool DescubrimientoIslas::EsDuplicado( const std::string & clave ) {
   if ( vistos.count( clave ) ) return true;
   vistos.insert( clave );
   vistosOrden.push_back( clave );
   if ( vistosOrden.size() > 500 ) {
      vistos.erase( vistosOrden.front() );
      vistosOrden.pop_front();
   }
   return false;
}

void DescubrimientoIslas::HiloEscucha() {
   char buf[512];
   while ( corriendo ) {
      std::string origenIp;
      int origenPuerto = 0;
      long n = udp.RecvFrom( buf, sizeof( buf ) - 1, origenIp, origenPuerto );
      if ( n <= 0 ) {
         if ( n < 0 ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
         continue;
      }
      std::string msg( buf, n );
      while ( !msg.empty() && ( msg.back() == '\n' || msg.back() == '\r' ) ) msg.pop_back();

      std::vector<std::string> p = ProtocoloIslas::Partir( msg, '/' );
      if ( p.empty() ) continue;

      if ( p[0] == "60" && p.size() == 5 ) {
         // 60/ID_MSG/id_isla/ip/puerto
         if ( !Digitos( p[1], 4, 4 ) || !IdIslaValido( p[2] ) || !IpValida( p[3] ) || !Digitos( p[4], 1, 5 ) ) continue;
         int puertoIsla = std::stoi( p[4] );
         if ( puertoIsla < 1 || puertoIsla > 65535 ) continue;
         if ( p[2] == miId ) continue;                       // eco propio
         if ( EsDuplicado( p[2] + "/" + p[1] ) ) continue;   // duplicado o bucle

         // La consulta TCP puede tardar: no se hace en el hilo que escucha
         std::string id = p[2], ip = p[3];
         std::thread( [this, id, ip, puertoIsla]() {
            servicio.IslaDescubierta( id, ip, puertoIsla );
         } ).detach();

      } else if ( p[0] == "61" && p.size() == 2 ) {
         if ( IdIslaValido( p[1] ) && p[1] != miId ) {
            servicio.IslaCaida( p[1] );
         }
      }
      // cualquier otro tipo se ignora
   }
}