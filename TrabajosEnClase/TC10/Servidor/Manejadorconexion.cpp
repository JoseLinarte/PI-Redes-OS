#include <iostream>
#include "Manejadorconexion.hpp"
#include "Httprequest.hpp"
#include "Httpresponse.hpp"
#include "Router.hpp"

std::string ManejadorConexion::LeerRequestCompleto( Socket & cliente ) {
   std::string acumulado;
   char buffer[ TAMANO_BUFFER ];

   while ( acumulado.size() < LIMITE_REQUEST ) {
      // Read() lanza runtime_error si read() devuelve -1 
      // esa excepcion se deja propagar hasta el try/catch de Atender().
      // Si el cliente cierra la conexion limpiamente, read() (y por lo
      // tanto Read()) devuelve 0 -- eso NO lanza excepcion.
      size_t leidos = cliente.Read( buffer, TAMANO_BUFFER );
      if ( leidos == 0 ) break;   // conexion cerrada por el cliente
      acumulado.append( buffer, leidos );

      // Ya tenemos los headers completos: revisamos si con eso alcanza
      // (GET sin body) o si falta leer el body segun Content-Length.
      size_t finHeaders = acumulado.find( "\r\n\r\n" );
      if ( finHeaders == std::string::npos ) continue;   // headers incompletos, seguir leyendo

      HttpRequest parcial = HttpRequest::Parsear( acumulado );
      if ( !parcial.valido ) break;   // se dejara que Router reporte el error 400

      auto it = parcial.headers.find( "content-length" );
      if ( it != parcial.headers.end() ) {
         try {
            size_t largo = (size_t)std::stoul( it->second );
            if ( parcial.cuerpo.size() >= largo ) break;   // ya llego todo el body
         } catch ( const std::exception & ) {
            break;   // Content-Length invalido: se corta aqui y el 400 lo reporta el Router
         }
      } else {
         break;   // sin Content-Length: se asume que no hay body (GET)
      }
   }
   return acumulado;
}

void ManejadorConexion::EscribirCompleto( Socket & cliente, const std::string & datos ) {
   size_t enviados = 0;
   while ( enviados < datos.size() ) {
      size_t escritos = cliente.Write( (const void *)( datos.data() + enviados ),datos.size() - enviados );
      if ( escritos == 0 ) break;   // no deberia pasar sin excepcion, pero por seguridad
      enviados += escritos;
   }
}

void ManejadorConexion::Atender( Socket cliente, ServicioProductos & servicio ) {
   try {
      cliente.SetReadTimeout( TIMEOUT_LECTURA_SEGUNDOS );

      std::string crudo = LeerRequestCompleto( cliente );
      if ( crudo.empty() ) {
         return;   // el destructor de 'cliente' cierra el socket
      }

      HttpRequest req = HttpRequest::Parsear( crudo );
      RespuestaServicio resp = Router::Despachar( req, servicio );
      std::string respuestaCruda = HttpResponse::Construir( resp );

      EscribirCompleto( cliente, respuestaCruda );
   } catch ( const std::exception & e ) {
      std::cerr << "[ManejadorConexion] Excepcion atendiendo conexion: " << e.what() << "\n";
   }
   // No hace falta llamar cliente.Close() explicitamente: el destructor
   // de Socket (via ~VSocket) lo hace al salir de este scope.
}