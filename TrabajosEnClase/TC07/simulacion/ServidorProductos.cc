/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  *  Servidor de productos - bodegas por bloques (version provisional)
  *  Modelo de atencion: HILOS
  *  Salida de datos: HTML especializado
  *
 **/

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <fstream>

#include "Socket.h"
#include "FileSystem.hpp"

#define PORT 8080
#define BUFSIZE 4096

static FileSystem * fsPtr = nullptr;
#define fs (*fsPtr)
static std::mutex fsMutex;      // protege el file system entre hilos concurrentes

struct InfoCatalogo {
   std::string categoria;
   int precio;
};
static std::map<std::string, InfoCatalogo> catalogo;


void CargarCatalogo() {
   catalogo[ "Refresco" ]    = { "Alimentos", 650 };
   catalogo[ "Papas" ]       = { "Alimentos", 900 };
   catalogo[ "Chocolate" ]   = { "Alimentos", 750 };
   catalogo[ "Bloque2x4" ]   = { "Construccion", 300 };
   catalogo[ "Bloque2x2" ]   = { "Construccion", 250 };
   catalogo[ "PlacaBase" ]   = { "Construccion", 1200 };
   catalogo[ "RuedaChica" ]  = { "Vehiculos", 400 };
   catalogo[ "Chasis" ]      = { "Vehiculos", 1500 };
   catalogo[ "MotorJuguete" ]= { "Vehiculos", 2200 };
}

void SembrarBodegasSiEsNuevo( bool archivoEraNuevo ) {

   if ( !archivoEraNuevo ) return;

   int32_t b1 = fs.crearBodega( "Bodega Central", "San Jose", 5000 );
   fs.agregarProducto( b1, "Refresco", 40 );
   fs.agregarProducto( b1, "Papas", 25 );
   fs.agregarProducto( b1, "Chocolate", 30 );
   fs.agregarProducto( b1, "Bloque2x4", 120 );

   int32_t b2 = fs.crearBodega( "Bodega Sur", "Cartago", 3000 );
   fs.agregarProducto( b2, "Bloque2x2", 95 );
   fs.agregarProducto( b2, "PlacaBase", 15 );
   fs.agregarProducto( b2, "RuedaChica", 60 );
   fs.agregarProducto( b2, "Chasis", 20 );
   fs.agregarProducto( b2, "MotorJuguete", 10 );

   std::cout << "[SERVIDOR] Bodegas de ejemplo sembradas (archivo nuevo)\n";

}


std::string UrlDecode( const std::string & texto ) {
   std::string resultado;
   for ( size_t i = 0; i < texto.size(); i++ ) {
      if ( '%' == texto[ i ] && i + 2 < texto.size() ) {
         int valor = std::stoi( texto.substr( i + 1, 2 ), nullptr, 16 );
         resultado += (char) valor;
         i += 2;
      } else if ( '+' == texto[ i ] ) {
         resultado += ' ';
      } else {
         resultado += texto[ i ];
      }
   }
   return resultado;
}

std::vector<std::string> SplitPorBarra( const std::string & texto ) {
   std::vector<std::string> partes;
   std::stringstream ss( texto );
   std::string token;
   while ( std::getline( ss, token, '/' ) ) {
      if ( !token.empty() ) partes.push_back( token );
   }
   return partes;
}

std::string HtmlPagina( const std::string & titulo, const std::string & cuerpo ) {
   std::string html;
   html += "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
   html += "<title>Bodegas - " + titulo + "</title></head><body>";
   html += "<h2>" + titulo + "</h2>";
   html += cuerpo;
   html += "</body></html>";
   return html;
}

std::string HttpResponder( int codigo, const std::string & textoCodigo, const std::string & html ) {
   std::ostringstream resp;
   resp << "HTTP/1.1 " << codigo << " " << textoCodigo << "\r\n"
        << "Content-Type: text/html; charset=utf-8\r\n"
        << "Content-Length: " << html.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << html;
   return resp.str();
}


std::string ManejarCategorias() {

   std::vector<std::string> categorias;
   for ( auto & entry : catalogo ) {
      bool yaEsta = false;
      for ( auto & c : categorias ) if ( c == entry.second.categoria ) yaEsta = true;
      if ( !yaEsta ) categorias.push_back( entry.second.categoria );
   }

   std::string filas;
   for ( auto & c : categorias ) {
      filas += "<li><a href=\"/productos/" + c + "\">" + c + "</a></li>";
   }

   return HtmlPagina( "Categorias disponibles", "<ul>" + filas + "</ul>" );

}

std::string ManejarProductos( const std::string & categoria ) {

   std::lock_guard<std::mutex> guard( fsMutex );

   std::string filas;
   for ( auto & entry : catalogo ) {
      if ( entry.second.categoria != categoria ) continue;

      auto ubicaciones = fs.buscarProductoEnTodasBodegas( entry.first );
      for ( auto & u : ubicaciones ) {
         filas += "<tr><td>" + entry.first + "</td><td>" + std::to_string( entry.second.precio ) +
                  "</td><td>" + u.nombreBodega + "</td><td>" + std::to_string( u.idBodega ) +
                  "</td><td>" + std::to_string( u.cantidad ) + "</td></tr>";
      }
   }

   std::string tabla =
      "<table border=\"1\" cellpadding=\"6\">"
      "<tr><th>Producto</th><th>Precio</th><th>Bodega</th><th>IdBodega</th><th>Stock</th></tr>"
      + filas + "</table>";

   return HtmlPagina( "Productos de " + categoria, tabla );

}

std::string ManejarAgregar( const std::string & nombreProducto, int32_t cantidad, int32_t idBodega ) {

   std::lock_guard<std::mutex> guard( fsMutex );

   std::string error;
   bool ok = fs.reservarProducto( idBodega, nombreProducto, cantidad, &error );

   if ( !ok ) {
      return HtmlPagina( "No se pudo agregar", "<p>ERROR: " + error + "</p>" );
   }

   int precio = 0;
   auto it = catalogo.find( nombreProducto );
   if ( catalogo.end() != it ) precio = it->second.precio;
   long subtotal = (long) precio * cantidad;

   std::string cuerpo;
   cuerpo += "<p>OK</p>";
   cuerpo += "<p>DATA|" + nombreProducto + "|" + std::to_string( cantidad ) + "|" +
             std::to_string( idBodega ) + "|" + std::to_string( precio ) + "|" +
             std::to_string( subtotal ) + "</p>";

   return HtmlPagina( "Producto agregado", cuerpo );

}

std::string ManejarFactura( const std::string & itemsCrudo ) {

   // itemsCrudo viene como "producto:cantidad:idBodega,producto:cantidad:idBodega,..."
   std::string filas;
   long total = 0;

   std::stringstream ss( itemsCrudo );
   std::string par;

   while ( std::getline( ss, par, ',' ) ) {

      size_t sep1 = par.find( ':' );
      size_t sep2 = par.find( ':', sep1 + 1 );
      if ( std::string::npos == sep1 || std::string::npos == sep2 ) continue;

      std::string nombreProducto = par.substr( 0, sep1 );
      int32_t cantidad = std::stoi( par.substr( sep1 + 1, sep2 - sep1 - 1 ) );
      int32_t idBodega = std::stoi( par.substr( sep2 + 1 ) );

      int precio = 0;
      auto it = catalogo.find( nombreProducto );
      if ( catalogo.end() != it ) precio = it->second.precio;

      long subtotal = (long) precio * cantidad;
      total += subtotal;

      filas += "<tr><td>" + nombreProducto + "</td><td>" + std::to_string( cantidad ) +
               "</td><td>" + std::to_string( idBodega ) + "</td><td>" + std::to_string( precio ) +
               "</td><td>" + std::to_string( subtotal ) + "</td></tr>";
   }

   std::string tabla =
      "<table border=\"1\" cellpadding=\"6\">"
      "<tr><th>Producto</th><th>Cantidad</th><th>IdBodega</th><th>Precio unit.</th><th>Subtotal</th></tr>"
      + filas + "</table>"
      + "<p><b>Total: " + std::to_string( total ) + "</b></p>";

   return HtmlPagina( "Factura proforma", tabla );

}


void task( VSocket * cliente ) {

   char buffer[ BUFSIZE ] = { 0 };
   cliente->Read( buffer, BUFSIZE - 1 );
   std::string request( buffer );

   std::string respuesta;

   size_t inicioPath = request.find( ' ' ) + 1;
   size_t finPath = request.find( ' ', inicioPath );
   std::string path = ( std::string::npos != finPath ) ? request.substr( inicioPath, finPath - inicioPath ) : "/";

   if ( 0 == path.find( "/categorias" ) ) {

      respuesta = HttpResponder( 200, "OK", ManejarCategorias() );

   } else if ( 0 == path.find( "/productos/" ) ) {

      std::string categoria = UrlDecode( path.substr( strlen( "/productos/" ) ) );
      respuesta = HttpResponder( 200, "OK", ManejarProductos( categoria ) );

   } else if ( 0 == path.find( "/agregar/" ) ) {

      std::string resto = path.substr( strlen( "/agregar/" ) );
      auto partes = SplitPorBarra( resto );

      if ( partes.size() < 3 ) {
         respuesta = HttpResponder( 400, "Bad Request",
            HtmlPagina( "Peticion invalida", "<p>Uso: /agregar/producto/cantidad/idBodega</p>" ) );
      } else {
         std::string nombreProducto = UrlDecode( partes[ 0 ] );
         int32_t cantidad = std::stoi( partes[ 1 ] );
         int32_t idBodega = std::stoi( partes[ 2 ] );
         respuesta = HttpResponder( 200, "OK", ManejarAgregar( nombreProducto, cantidad, idBodega ) );
      }

   } else if ( 0 == path.find( "/factura" ) ) {

      std::string itemsCrudo;
      size_t q = path.find( "?items=" );
      if ( std::string::npos != q ) {
         itemsCrudo = UrlDecode( path.substr( q + strlen( "?items=" ) ) );
      }
      respuesta = HttpResponder( 200, "OK", ManejarFactura( itemsCrudo ) );

   } else if ( 0 == path.find( "/shutdown" ) ) {

      respuesta = HttpResponder( 200, "OK", HtmlPagina( "Cerrando", "<p>Servidor apagandose...</p>" ) );
      cliente->Write( respuesta.c_str() );
      cliente->Close();
      delete cliente;
      exit( 0 );

   } else {

      respuesta = HttpResponder( 404, "Not Found", HtmlPagina( "No encontrado", "<p>Ruta invalida.</p>" ) );

   }

   cliente->Write( respuesta.c_str() );
   cliente->Close();
   delete cliente;

}


int main() {

   std::ifstream pruebaExiste( "bodegas.dat" );
   bool archivoEraNuevo = !pruebaExiste.good();
   pruebaExiste.close();

   CargarCatalogo();
   fsPtr = new FileSystem( "bodegas.dat" );
   SembrarBodegasSiEsNuevo( archivoEraNuevo );

   VSocket * s1 = new Socket( 's' );

   s1->Bind( PORT );
   s1->MarkPassive( 10 );

   std::cout << "[SERVIDOR PRODUCTOS] Escuchando en el puerto " << PORT << " (modelo: hilos)\n";

   for ( ; ; ) {
      VSocket * cliente = s1->AcceptConnection();
      std::thread worker( task, cliente );
      worker.detach();
   }

   return 0;

}