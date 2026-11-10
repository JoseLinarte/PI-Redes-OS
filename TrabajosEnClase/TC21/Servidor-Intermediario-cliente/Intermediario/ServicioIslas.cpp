#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include "ServicioProductosDistribuido.hpp"
#include "ProtocoloIslas.hpp"
#include "Bitacora.hpp"

static std::string Decimal2( double v ) {
   char buf[32];
   std::snprintf( buf, sizeof( buf ), "%.2f", v );
   return buf;
}

bool ServicioProductosDistribuido::BuscarProductoProtocolo( const std::string & producto, std::string & categoria, std::string & descripcion, int & stock ) const {
   bool encontrado = false;
   stock = 0;
   for ( const InfoServidor & s : registro.Activos() ) {
      for ( const std::string & c : s.categorias ) {
         std::vector<FilaProducto> filas;
         if ( !ConsultarProductos( s, c, filas ) ) continue;
         for ( const FilaProducto & f : filas ) {
            if ( ProtocoloIslas::ANombre( f.descripcion ) != producto ) continue;
            if ( !encontrado ) {
               categoria = f.categoria;
               descripcion = f.descripcion;
            }
            encontrado = true;
            int disponible = std::atoi( f.cantidad.c_str() );
            if ( disponible > stock ) stock = disponible;
            break;
         }
      }
   }
   return encontrado;
}

std::string ServicioProductosDistribuido::RespuestaCategorias() const {
   std::vector<std::string> cats;
   for ( const InfoServidor & s : registro.Activos() ) {
      for ( const std::string & c : s.categorias ) {
         std::string n = ProtocoloIslas::ANombre( c );
         if ( !ProtocoloIslas::CategoriaValida( n ) ) continue;
         if ( std::find( cats.begin(), cats.end(), n ) == cats.end() ) cats.push_back( n );
      }
   }
   if ( cats.empty() ) return "01";
   std::string r = "01/";
   for ( size_t i = 0; i < cats.size(); i++ ) {
      if ( i > 0 ) r += ",";
      r += cats[i];
   }
   return r;
}

std::string ServicioProductosDistribuido::RespuestaProductos( const std::string & categoria ) const {
   std::vector<FilaProducto> filas;
   bool existe = false;
   for ( const InfoServidor & s : registro.Activos() ) {
      for ( const std::string & c : s.categorias ) {
         if ( ProtocoloIslas::ANombre( c ) != categoria ) continue;
         existe = true;
         ConsultarProductos( s, c, filas );
         break;
      }
   }
   if ( !existe ) return "03/" + categoria;
   std::string lista;
   int n = 0;
   for ( const FilaProducto & f : filas ) {
      std::string nombre = ProtocoloIslas::ANombre( f.descripcion );
      if ( !ProtocoloIslas::CategoriaValida( nombre ) ) continue;
      if ( n > 0 ) lista += ";";
      lista += nombre + "," + ProtocoloIslas::Precio( f.precio ) + "," + std::to_string( std::atoi( f.cantidad.c_str() ) );
      n++;
   }
   return "01/" + std::to_string( n ) + "/" + lista;
}

std::string ServicioProductosDistribuido::RespuestaReserva( const std::string & producto, const std::string & cantidadTexto ) const {
   if ( !ProtocoloIslas::CategoriaValida( producto ) || !ProtocoloIslas::SoloDigitos( cantidadTexto, 1, 5 ) ) return "02/102";
   int cantidad = std::atoi( cantidadTexto.c_str() );
   if ( cantidad <= 0 ) return "02/102";
   std::string categoria;
   std::string descripcion;
   int stock = 0;
   if ( !BuscarProductoProtocolo( producto, categoria, descripcion, stock ) ) return "03/" + producto;
   std::lock_guard<std::mutex> lock( mtxReservas );
   std::time_t ahora = std::time( nullptr );
   reservas.erase( std::remove_if( reservas.begin(), reservas.end(), [ahora]( const ReservaIsla & r ) { return r.expira <= ahora; } ), reservas.end() );
   int reservado = 0;
   for ( const ReservaIsla & r : reservas ) {
      if ( r.clave == producto ) reservado += r.cantidad;
   }
   if ( cantidad > stock - reservado ) return "02/103";
   ReservaIsla nueva;
   nueva.clave = producto;
   nueva.categoria = categoria;
   nueva.descripcion = descripcion;
   nueva.cantidad = cantidad;
   nueva.expira = ahora + 60;
   reservas.push_back( nueva );
   return "01";
}

std::string ServicioProductosDistribuido::RespuestaCompra( const std::vector<std::string> & p ) const {
   if ( !ProtocoloIslas::SoloDigitos( p[1], 1, 3 ) ) return "02/102";
   size_t esperados = std::stoul( p[1] );
   std::vector<std::string> piezas = ProtocoloIslas::Partir( p[2], ';' );
   if ( esperados == 0 || piezas.size() != esperados ) return "02/100";
   std::vector<std::string> nombres;
   std::vector<int> cantidades;
   for ( const std::string & pieza : piezas ) {
      std::vector<std::string> campos = ProtocoloIslas::Partir( pieza, ',' );
      if ( campos.size() != 2 ) return "02/100";
      std::string nombre = ProtocoloIslas::Minusculas( campos[0] );
      if ( !ProtocoloIslas::CategoriaValida( nombre ) || !ProtocoloIslas::SoloDigitos( campos[1], 1, 5 ) ) return "02/102";
      int cantidad = std::atoi( campos[1].c_str() );
      if ( cantidad <= 0 ) return "02/102";
      nombres.push_back( nombre );
      cantidades.push_back( cantidad );
   }
   std::vector<SolicitudItem> items;
   {
      std::lock_guard<std::mutex> lock( mtxReservas );
      std::time_t ahora = std::time( nullptr );
      reservas.erase( std::remove_if( reservas.begin(), reservas.end(), [ahora]( const ReservaIsla & r ) { return r.expira <= ahora; } ), reservas.end() );
      std::vector<ReservaIsla> copia = reservas;
      for ( size_t i = 0; i < nombres.size(); i++ ) {
         bool hallada = false;
         for ( ReservaIsla & r : copia ) {
            if ( r.clave != nombres[i] || r.cantidad < cantidades[i] ) continue;
            r.cantidad -= cantidades[i];
            SolicitudItem it;
            it.categoria = r.categoria;
            it.descripcion = r.descripcion;
            it.cantidad = cantidades[i];
            items.push_back( it );
            hallada = true;
            break;
         }
         if ( !hallada ) return "02/104";
      }
      copia.erase( std::remove_if( copia.begin(), copia.end(), []( const ReservaIsla & r ) { return r.cantidad <= 0; } ), copia.end() );
      reservas = copia;
   }
   Proforma pf = ComprarItems( items );
   if ( !pf.EsValida() ) return "02/103";
   std::string detalle;
   for ( size_t i = 0; i < pf.Lineas().size(); i++ ) {
      const LineaProforma & l = pf.Lineas()[i];
      if ( i > 0 ) detalle += ";";
      detalle += ProtocoloIslas::ANombre( l.descripcion ) + "," + std::to_string( l.cantidad ) + "," + Decimal2( l.Subtotal() );
   }
   return "01/" + Decimal2( pf.Total() ) + "/" + std::to_string( pf.Lineas().size() ) + "/" + detalle;
}

std::string ServicioProductosDistribuido::AtenderIsla( const std::string & mensaje ) const {
   std::string msg = mensaje;
   while ( !msg.empty() && ( msg.back() == '\n' || msg.back() == '\r' || msg.back() == ' ' ) ) msg.pop_back();
   std::vector<std::string> p = ProtocoloIslas::Partir( msg, '/' );
   if ( p.empty() || !ProtocoloIslas::SoloDigitos( p[0], 2, 2 ) ) return "02/100";
   const std::string & tipo = p[0];
   if ( tipo == "10" ) {
      if ( p.size() != 1 ) return "02/100";
      return RespuestaCategorias();
   }
   if ( tipo == "20" ) {
      if ( p.size() != 2 ) return "02/100";
      std::string categoria = ProtocoloIslas::Minusculas( p[1] );
      if ( !ProtocoloIslas::CategoriaValida( categoria ) ) return "02/102";
      return RespuestaProductos( categoria );
   }
   if ( tipo == "50" ) {
      if ( p.size() != 3 ) return "02/100";
      return RespuestaReserva( ProtocoloIslas::Minusculas( p[1] ), p[2] );
   }
   if ( tipo == "40" ) {
      if ( p.size() != 3 ) return "02/100";
      return RespuestaCompra( p );
   }
   if ( tipo == "61" ) {
      if ( p.size() != 2 ) return "02/100";
      IslaCaida( p[1] );
      return "01";
   }
   return "02/101";
}