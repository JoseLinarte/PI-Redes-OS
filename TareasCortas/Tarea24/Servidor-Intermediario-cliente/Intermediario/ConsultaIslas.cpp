#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <thread>
#include "ServicioProductosDistribuido.hpp"
#include "ProtocoloIslas.hpp"
#include "Bitacora.hpp"
#include "Texto.hpp"

static bool ParsearCompraIsla( const std::string & resp, const InfoIsla & isla, const std::vector<SolicitudItem> & items, Proforma & destino ) {
   std::vector<std::string> p = ProtocoloIslas::Partir( resp, '/' );
   if ( p.size() < 4 || p[0] != "01" ) return false;
   for ( const std::string & pieza : ProtocoloIslas::Partir( p[3], ';' ) ) {
      std::vector<std::string> c = ProtocoloIslas::Partir( pieza, ',' );
      if ( c.size() != 3 ) return false;
      LineaProforma l;
      l.bodega = isla.id;
      l.categoria = "";
      for ( const SolicitudItem & it : items ) {
         if ( ProtocoloIslas::ANombre( it.descripcion ) == c[0] ) {
            l.categoria = it.categoria;
            break;
         }
      }
      l.descripcion = c[0];
      l.cantidad = std::atoi( c[1].c_str() );
      if ( l.cantidad <= 0 ) return false;
      l.precioUnitario = std::atof( c[2].c_str() ) / l.cantidad;
      destino.AgregarLinea( l );
   }
   return true;
}

bool ServicioProductosDistribuido::ConsultarIsla( const InfoIsla & isla, const std::string & mensaje, int intentos, std::string & respuesta ) const {
   for ( int i = 0; i < intentos; i++ ) {
      if ( ProtocoloIslas::Enviar( isla.ip, isla.puerto, mensaje, respuesta, 5 ) ) {
         if ( mensaje != "10" ) {
            std::string tipo = ( mensaje.compare( 0, 2, "40" ) == 0 ) ? "PROFORMA" : "CONSULTA";
            Bitacora::Registrar( tipo, "Solicitud a " + isla.id + ": " + mensaje.substr( 0, 80 ) + " -> " + respuesta.substr( 0, 60 ), isla.id );
         }
         return true;
      }
   }
   if ( islas.MarcarInactiva( isla.id ) ) {
      Bitacora::Registrar( "INACTIVO", "Isla inactiva por error de consulta: " + isla.id + " (" + isla.ip + ":" + std::to_string( isla.puerto ) + ")", isla.id );
   }
   return false;
}

bool ServicioProductosDistribuido::ConsultarProductosIsla( const InfoIsla & isla, const std::string & categoria, std::vector<FilaProducto> & filas ) const {
   std::string resp;
   if ( !ConsultarIsla( isla, "20/" + categoria, 3, resp ) ) return false;
   std::vector<std::string> p = ProtocoloIslas::Partir( resp, '/' );
   if ( p.size() < 3 || p[0] != "01" ) return false;
   for ( const std::string & pieza : ProtocoloIslas::Partir( p[2], ';' ) ) {
      std::vector<std::string> c = ProtocoloIslas::Partir( pieza, ',' );
      if ( c.size() != 3 ) continue;
      FilaProducto f;
      f.intermediario = isla.id;
      f.bodega = "remota";
      f.categoria = categoria;
      f.descripcion = c[0];
      f.precio = c[1];
      f.cantidad = c[2];
      filas.push_back( f );
   }
   return true;
}

bool ServicioProductosDistribuido::ExisteLocal( const SolicitudItem & item ) const {
   for ( const InfoServidor & s : registro.ServidoresDeCategoria( item.categoria ) ) {
      std::vector<FilaProducto> filas;
      if ( !ConsultarProductos( s, item.categoria, filas ) ) continue;
      for ( const FilaProducto & f : filas ) {
         if ( IgualesSinMayusculas( f.descripcion, item.descripcion ) ) return true;
      }
   }
   return false;
}

std::vector<InfoIsla> ServicioProductosDistribuido::IslasDeCategoria( const std::string & categoria ) const {
   std::string nombre = ProtocoloIslas::ANombre( categoria );
   std::vector<InfoIsla> r;
   for ( const InfoIsla & isla : islas.Activas() ) {
      for ( const std::string & c : isla.categorias ) {
         if ( ProtocoloIslas::ANombre( c ) == nombre ) {
            r.push_back( isla );
            break;
         }
      }
   }
   return r;
}

Proforma ServicioProductosDistribuido::ComprarConIslas( const std::vector<SolicitudItem> & items ) const {
   Proforma resultado;
   std::vector<SolicitudItem> locales;
   std::map<std::string, std::vector<SolicitudItem>> remotos;
   std::map<std::string, InfoIsla> infoIslas;
   for ( const SolicitudItem & it : items ) {
      if ( ExisteLocal( it ) ) {
         locales.push_back( it );
         continue;
      }
      std::string producto = ProtocoloIslas::ANombre( it.descripcion );
      bool candidata = false;
      bool reservado = false;
      for ( const InfoIsla & isla : IslasDeCategoria( it.categoria ) ) {
         candidata = true;
         std::string resp;
         if ( !ConsultarIsla( isla, "50/" + producto + "/" + std::to_string( it.cantidad ), 3, resp ) ) continue;
         if ( resp.compare( 0, 2, "01" ) == 0 ) {
            remotos[isla.id].push_back( it );
            infoIslas[isla.id] = isla;
            reservado = true;
            break;
         }
      }
      if ( reservado ) continue;
      if ( candidata ) resultado.AgregarError( "No se pudo reservar '" + it.descripcion + "' en otras islas (stock insuficiente o inexistente)" );
      else locales.push_back( it );
   }
   if ( !resultado.EsValida() ) return resultado;

   if ( !locales.empty() ) {
      Proforma pl = ComprarItems( locales );
      for ( const LineaProforma & l : pl.Lineas() ) resultado.AgregarLinea( l );
      for ( const std::string & e : pl.Errores() ) resultado.AgregarError( e );
      if ( !pl.EsValida() ) return resultado;
   }

   for ( const auto & par : remotos ) {
      const InfoIsla & isla = infoIslas.at( par.first );
      std::string detalle;
      for ( size_t i = 0; i < par.second.size(); i++ ) {
         if ( i > 0 ) detalle += ";";
         detalle += ProtocoloIslas::ANombre( par.second[i].descripcion ) + "," + std::to_string( par.second[i].cantidad );
      }
      std::string resp;
      if ( !ConsultarIsla( isla, "40/" + std::to_string( par.second.size() ) + "/" + detalle, 1, resp ) ) {
         resultado.AgregarError( "No se pudo contactar a la isla " + isla.id + ": sus productos NO se compraron" );
         continue;
      }
      if ( !ParsearCompraIsla( resp, isla, par.second, resultado ) ) {
         resultado.AgregarError( "La isla " + isla.id + " rechazo la compra (" + resp.substr( 0, 40 ) + ")" );
      }
   }
   return resultado;
}

void ServicioProductosDistribuido::IniciarHilosIslas( int miIsla, int segundos ) {
   for ( int n = 1; n <= 6; n++ ) {
      if ( n == miIsla ) continue;
      char buf[16];
      std::snprintf( buf, sizeof( buf ), "INT_%02d", n );
      std::string idIsla = buf;
      std::thread( [this, idIsla, segundos]() {
         while ( true ) {
            std::this_thread::sleep_for( std::chrono::seconds( segundos ) );
            InfoIsla isla;
            if ( !islas.Obtener( idIsla, isla ) || !isla.activa ) continue;
            std::string resp;
            std::vector<std::string> cats;
            if ( ConsultarIsla( isla, "10", 3, resp ) && ProtocoloIslas::ParsearCategorias( resp, cats ) ) {
               islas.FijarCategorias( idIsla, cats );
            }
         }
      } ).detach();
   }
}