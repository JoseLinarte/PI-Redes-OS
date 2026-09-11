/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  *  Cliente interactivo - adaptado al servidor de bodegas
  *
 **/

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cctype>

#include "Socket.h"

#define MAXBUF 4096

struct ItemCarrito {
   std::string nombreProducto;
   int32_t     cantidad;
   int32_t     idBodega;
   int         precio;
   long        subtotal;
};

std::string UrlEncode( const std::string & texto ) {
   std::ostringstream out;
   for ( unsigned char c : texto ) {
      if ( isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) {
         out << c;
      } else {
         out << '%' << std::uppercase << std::hex << (int) c << std::nouppercase << std::dec;
      }
   }
   return out.str();
}


std::string HacerPeticion( const char * host, int puerto, const std::string & path ) {

   VSocket * conexion = new Socket( 's' );

   std::string request =
      "GET " + path + " HTTP/1.1\r\n"
      "Host: " + std::string( host ) + "\r\n"
      "Connection: close\r\n\r\n";

   conexion->Connect( host, puerto );
   conexion->Write( request.c_str() );

   std::string respuesta;
   char buffer[ MAXBUF ];
   size_t leidos;
   while ( ( leidos = conexion->Read( buffer, MAXBUF - 1 ) ) > 0 ) {
      buffer[ leidos ] = '\0';
      respuesta += buffer;
   }

   delete conexion;

   size_t finEncabezados = respuesta.find( "\r\n\r\n" );
   if ( std::string::npos != finEncabezados ) {
      respuesta = respuesta.substr( finEncabezados + 4 );
   }

   return respuesta;

}

std::vector<std::string> ExtraerEntreEtiquetas( const std::string & html, const std::string & tagApertura, const std::string & tagCierre ) {

   std::vector<std::string> resultado;
   size_t pos = 0;

   while ( true ) {
      size_t ini = html.find( tagApertura, pos );
      if ( std::string::npos == ini ) break;
      ini += tagApertura.size();
      size_t fin = html.find( tagCierre, ini );
      if ( std::string::npos == fin ) break;
      resultado.push_back( html.substr( ini, fin - ini ) );
      pos = fin + tagCierre.size();
   }

   return resultado;

}

void MostrarCategorias( const std::string & html ) {

   auto items = ExtraerEntreEtiquetas( html, "<li>", "</li>" );
   std::cout << "\nCategorias disponibles:\n";
   for ( auto & item : items ) {
      size_t inicioTexto = item.find( '>' );
      std::string nombre = item;
      if ( std::string::npos != inicioTexto ) {
         size_t finTexto = item.find( '<', inicioTexto );
         nombre = item.substr( inicioTexto + 1, finTexto - inicioTexto - 1 );
      }
      std::cout << "  - " << nombre << "\n";
   }

}

void MostrarTabla( const std::string & html, int columnas ) {

   auto celdas = ExtraerEntreEtiquetas( html, "<td>", "</td>" );
   for ( size_t i = 0; i < celdas.size(); i++ ) {
      std::cout << celdas[ i ];
      std::cout << ( ( (int) i % columnas == columnas - 1 ) ? "\n" : "\t| " );
   }
   std::cout << std::endl;

   auto negritas = ExtraerEntreEtiquetas( html, "<b>", "</b>" );
   for ( auto & b : negritas ) std::cout << b << "\n";

}


bool ParsearRespuestaAgregar( const std::string & html, ItemCarrito * item ) {

   auto parrafos = ExtraerEntreEtiquetas( html, "<p>", "</p>" );

   bool huboOk = false;
   for ( auto & p : parrafos ) if ( 0 == p.find( "OK" ) ) huboOk = true;
   if ( !huboOk ) {
      for ( auto & p : parrafos ) if ( 0 == p.find( "ERROR" ) ) std::cout << p << "\n";
      return false;
   }

   for ( auto & p : parrafos ) {
      if ( 0 != p.find( "DATA|" ) ) continue;

      std::vector<std::string> partes;
      std::stringstream ss( p.substr( 5 ) );
      std::string token;
      while ( std::getline( ss, token, '|' ) ) partes.push_back( token );

      if ( partes.size() < 5 ) return false;

      item->nombreProducto = partes[ 0 ];
      item->cantidad       = std::stoi( partes[ 1 ] );
      item->idBodega       = std::stoi( partes[ 2 ] );
      item->precio         = std::stoi( partes[ 3 ] );
      item->subtotal       = std::stol( partes[ 4 ] );
      return true;
   }

   return false;

}


int main() {

   const char * host = "127.0.0.1";
   int puerto = 8080;

   std::vector<ItemCarrito> carrito;

   bool corriendo = true;

   std::cout << "=== Bodegas - Cliente ===\n";
   std::cout << "1) Listar categorias\n";
   std::cout << "2) Ver productos de una categoria\n";
   std::cout << "3) Agregar producto al carrito\n";
   std::cout << "4) Pedir factura proforma\n";
   std::cout << "5) Salir\n";

   while ( corriendo ) {

      std::cout << "\nComando: ";
      std::string comando;
      std::getline( std::cin, comando );

      if ( "1" == comando ) {

         std::string resp = HacerPeticion( host, puerto, "/categorias" );
         MostrarCategorias( resp );

      } else if ( "2" == comando ) {

         std::cout << "Nombre de la categoria: ";
         std::string categoria;
         std::getline( std::cin, categoria );

         std::string resp = HacerPeticion( host, puerto, "/productos/" + UrlEncode( categoria ) );
         std::cout << "\nProducto\t| Precio\t| Bodega\t| IdBodega\t| Stock\n";
         MostrarTabla( resp, 5 );

      } else if ( "3" == comando ) {

         std::cout << "Nombre del producto: ";
         std::string nombreProducto;
         std::getline( std::cin, nombreProducto );

         std::cout << "Cantidad: ";
         std::string cantStr;
         std::getline( std::cin, cantStr );

         std::cout << "Id de la bodega (columna IdBodega de la tabla anterior): ";
         std::string idStr;
         std::getline( std::cin, idStr );

         std::string path = "/agregar/" + UrlEncode( nombreProducto ) + "/" + cantStr + "/" + idStr;
         std::string resp = HacerPeticion( host, puerto, path );

         ItemCarrito item;
         if ( ParsearRespuestaAgregar( resp, &item ) ) {
            carrito.push_back( item );
            std::cout << "Agregado: " << item.nombreProducto << " x" << item.cantidad
                      << " (subtotal " << item.subtotal << ")\n";
         }

      } else if ( "4" == comando ) {

         if ( carrito.empty() ) {
            std::cout << "El carrito esta vacio.\n";
            continue;
         }

         std::ostringstream items;
         for ( size_t i = 0; i < carrito.size(); i++ ) {
            if ( i > 0 ) items << ",";
            items << carrito[ i ].nombreProducto << ":" << carrito[ i ].cantidad << ":" << carrito[ i ].idBodega;
         }

         std::string resp = HacerPeticion( host, puerto, "/factura?items=" + UrlEncode( items.str() ) );
         std::cout << "\nProducto\t| Cantidad\t| IdBodega\t| Precio unit.\t| Subtotal\n";
         MostrarTabla( resp, 5 );

         carrito.clear();

      } else if ( "5" == comando ) {

         std::cout << "Saliendo...\n";
         corriendo = false;

      } else {

         std::cout << "Comando invalido.\n";

      }

   }

   return 0;

}