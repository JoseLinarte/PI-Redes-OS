#include <chrono>
#include <cstdlib>
#include <exception>
#include <memory>
#include <thread>
#include "ServidorIslas.hpp"
#include "ProtocoloIslas.hpp"
#include "Bitacora.hpp"

static std::string Recortar( std::string t ) {
   while ( !t.empty() && ( t.back() == '\n' || t.back() == '\r' || t.back() == ' ' ) ) t.pop_back();
   return t;
}

static bool Completo( const std::string & m ) {
   std::vector<std::string> p = ProtocoloIslas::Partir( Recortar( m ), '/' );
   if ( p.empty() ) return false;
   if ( p[0] == "20" || p[0] == "61" ) return p.size() >= 2;
   if ( p[0] == "50" ) return p.size() >= 3;
   if ( p[0] == "40" ) {
      if ( p.size() < 3 ) return false;
      return ProtocoloIslas::Partir( p[2], ';' ).size() >= (size_t)std::atoi( p[1].c_str() );
   }
   return true;
}

ServidorIslas::ServidorIslas( ServicioProductosDistribuido & servicio, int puerto, const std::string & ip ) : servicio( servicio ), puerto( puerto ), ip( ip ), escucha( 's', false ) {}
bool ServidorIslas::Iniciar() {
   try {
      if ( ip.empty() ) escucha.Bind( puerto );
      else escucha.BindDireccion( ip, puerto );
      escucha.Listen( 16 );
   } catch ( const std::exception & e ) {
      Bitacora::Registrar( "ERROR", std::string( "No se pudo iniciar el servidor de islas (TCP " ) + std::to_string( puerto ) + "): " + e.what(), "intermediario" );
      return false;
   }
   std::thread( [this]() { Aceptar(); } ).detach();
   Bitacora::Registrar( "INFO", "Servidor de islas escuchando en TCP " + std::to_string( puerto ), "intermediario" );
   return true;
}

void ServidorIslas::Aceptar() {
   while ( true ) {
      try {
         std::shared_ptr<Socket> conexion = std::make_shared<Socket>( escucha.Accept() );
         std::thread( [this, conexion]() { Atender( *conexion ); } ).detach();
      } catch ( const std::exception & ) {
         std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      }
   }
}

void ServidorIslas::Atender( Socket & conexion ) {
   try {
      conexion.SetReadTimeout( 5 );
      std::string msg;
      char buf[2048];
      while ( msg.size() < 65536 ) {
         size_t n = 0;
         try {
            n = conexion.Read( buf, sizeof( buf ) );
         } catch ( const std::exception & ) {
            break;
         }
         if ( n == 0 ) break;
         msg.append( buf, n );
         if ( Completo( msg ) ) break;
      }
      if ( msg.empty() ) return;
      std::string respuesta = servicio.AtenderIsla( msg );
      conexion.Write( respuesta.data(), respuesta.size() );
      std::string tipo = "CONSULTA";
      if ( respuesta.compare( 0, 2, "02" ) == 0 ) tipo = "ERROR";
      else if ( msg.compare( 0, 2, "40" ) == 0 ) tipo = "PROFORMA";
      Bitacora::Registrar( tipo, "Solicitud de otra isla: " + Recortar( msg ).substr( 0, 80 ) + " -> " + respuesta.substr( 0, 60 ), "isla" );
   } catch ( const std::exception & ) {
   }
}