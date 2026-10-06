#include <iostream>
#include "ManejadorIntermediario.hpp"
#include "Httprequest.hpp"

// Misma logica que ManejadorConexion::LeerRequestCompleto del servidor de productos
std::string ManejadorIntermediario::LeerRequestCompleto( Socket & cliente ) {
   std::string acumulado;
   char buffer[ TAMANO_BUFFER ];

   while ( acumulado.size() < LIMITE_REQUEST ) {
      size_t leidos = cliente.Read( buffer, TAMANO_BUFFER );
      if ( leidos == 0 ) break;
      acumulado.append( buffer, leidos );

      if ( acumulado.find( "\r\n\r\n" ) == std::string::npos ) continue;

      HttpRequest parcial = HttpRequest::Parsear( acumulado );
      if ( !parcial.valido ) break;

      auto it = parcial.headers.find( "content-length" );
      if ( it == parcial.headers.end() ) break;   // sin body (GET)
      try {
         size_t largo = (size_t)std::stoul( it->second );
         if ( parcial.cuerpo.size() >= largo ) break;
      } catch ( const std::exception & ) {
         break;
      }
   }
   return acumulado;
}

void ManejadorIntermediario::EscribirCompleto( Socket & s, const std::string & datos ) {
   size_t enviados = 0;
   while ( enviados < datos.size() ) {
      size_t n = s.Write( (const void *)( datos.data() + enviados ), datos.size() - enviados );
      if ( n == 0 ) break;
      enviados += n;
   }
}

// Abre una conexion nueva al servidor de productos, le manda el reques
// y lee hasta que cierre la conexion y el servidor de productos siempre
// responde con "Connection: close").
std::string ManejadorIntermediario::ReenviarAProductos( const std::string & request,const std::string & host,const std::string & puerto ) {
   Socket s( 's', false );
   s.SetReadTimeout( TIMEOUT_SEGUNDOS );
   s.Connect( host.c_str(), puerto.c_str() );   // version getaddrinfo
   EscribirCompleto( s, request );

   std::string respuesta;
   char buffer[ TAMANO_BUFFER ];
   size_t leidos;
   while ( ( leidos = s.Read( buffer, TAMANO_BUFFER ) ) > 0 ) {
      respuesta.append( buffer, leidos );
   }
   return respuesta;
}

std::string ManejadorIntermediario::RespuestaError( int codigo, const std::string & frase, const std::string & detalle ) {
   std::string cuerpo = "<!DOCTYPE html>\n<HTML><BODY><H1>Error " + std::to_string( codigo )+ "</H1><P>" + detalle + "</P></BODY></HTML>\n";
   return "HTTP/1.1 " + std::to_string( codigo ) + " " + frase + "\r\n"
          "Content-Type: text/html; charset=utf-8\r\n"
          "Content-Length: " + std::to_string( cuerpo.size() ) + "\r\n"
          "Connection: close\r\n\r\n" + cuerpo;
}

void ManejadorIntermediario::Atender( Socket cliente, std::string hostProductos, std::string puertoProductos ) {
   try {
      cliente.SetReadTimeout( TIMEOUT_SEGUNDOS );

      std::string request = LeerRequestCompleto( cliente );
      if ( request.empty() ) return;

      std::string respuesta;
      try {
         respuesta = ReenviarAProductos( request, hostProductos, puertoProductos );
         if ( respuesta.empty() ) {
            respuesta = RespuestaError( 502, "Bad Gateway", "El servidor de productos no devolvio respuesta" );
         }
      } catch ( const std::exception & e ) {
         std::cerr << "[Intermediario] Fallo hablando con productos: " << e.what() << "\n";
         respuesta = RespuestaError( 502, "Bad Gateway", "No se pudo contactar al servidor de productos" );
      }

      EscribirCompleto( cliente, respuesta );
   } catch ( const std::exception & e ) {
      std::cerr << "[Intermediario] Excepcion atendiendo conexion: " << e.what() << "\n";
   }
}