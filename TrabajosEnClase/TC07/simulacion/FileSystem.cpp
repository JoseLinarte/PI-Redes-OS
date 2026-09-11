#include "FileSystem.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

void BloqueControl::serializar( char * buffer ) const {
   memset( buffer, 0, TAMANO_BLOQUE );
   memcpy( buffer, &cantidadBodegas, 4 );
}

void BloqueControl::deserializar( const char * buffer ) {
   memcpy( &cantidadBodegas, buffer, 4 );
}

void BloqueBodega::serializar( char * buffer ) const {
   memset( buffer, 0, TAMANO_BLOQUE );
   int off = 0;
   memcpy( buffer + off, &idBodega, 4 ); off += 4;
   memcpy( buffer + off, nombre, 28 ); off += 28;
   memcpy( buffer + off, ubicacion, 24 ); off += 24;
   memcpy( buffer + off, &capacidadMaxima, 4 ); off += 4;
   memcpy( buffer + off, &cantidadProductos, 4 ); off += 4;
   for ( int i = 0; i < MAX_PRODUCTOS_BODEGA; i++ ) {
      memcpy( buffer + off, productos[ i ].nombre, 16 ); off += 16;
      memcpy( buffer + off, &productos[ i ].cantidad, 4 ); off += 4;
   }
   // quedan 12 bytes de relleno sin usar al final del bloque
}

void BloqueBodega::deserializar( const char * buffer ) {
   int off = 0;
   memcpy( &idBodega, buffer + off, 4 ); off += 4;
   memcpy( nombre, buffer + off, 28 ); off += 28;
   memcpy( ubicacion, buffer + off, 24 ); off += 24;
   memcpy( &capacidadMaxima, buffer + off, 4 ); off += 4;
   memcpy( &cantidadProductos, buffer + off, 4 ); off += 4;
   for ( int i = 0; i < MAX_PRODUCTOS_BODEGA; i++ ) {
      memcpy( productos[ i ].nombre, buffer + off, 16 ); off += 16;
      memcpy( &productos[ i ].cantidad, buffer + off, 4 ); off += 4;
   }
}


FileSystem::FileSystem( const std::string & archivo, bool * eraNuevo ) {

   disco.open( archivo, std::ios::in | std::ios::out | std::ios::binary );

   if ( !disco.is_open() ) {
      disco.open( archivo, std::ios::out | std::ios::binary );
      disco.close();
      disco.open( archivo, std::ios::in | std::ios::out | std::ios::binary );
      inicializarArchivo();
      if ( eraNuevo ) *eraNuevo = true;
   } else {
      char buffer[ TAMANO_BLOQUE ];
      leerBloque( BLOQUE_CONTROL, buffer );
      control.deserializar( buffer );
      if ( eraNuevo ) *eraNuevo = false;
   }

}

FileSystem::~FileSystem() {
   if ( disco.is_open() ) disco.close();
}

void FileSystem::inicializarArchivo() {
   control = BloqueControl{};
   guardarControl();
}

void FileSystem::leerBloque( int32_t numeroBloque, char * buffer ) {
   disco.seekg( (long) numeroBloque * TAMANO_BLOQUE );
   disco.read( buffer, TAMANO_BLOQUE );
}

void FileSystem::escribirBloque( int32_t numeroBloque, const char * buffer ) {
   disco.seekp( (long) numeroBloque * TAMANO_BLOQUE );
   disco.write( buffer, TAMANO_BLOQUE );
   disco.flush();
}

void FileSystem::guardarControl() {
   char buffer[ TAMANO_BLOQUE ];
   control.serializar( buffer );
   escribirBloque( BLOQUE_CONTROL, buffer );
}



int32_t FileSystem::crearBodega( const std::string & nombre, const std::string & ubicacion, int32_t capacidadMaxima ) {

   int32_t indiceBloque = control.cantidadBodegas + 1;   // +1 porque el bloque 0 es control

   BloqueBodega bodega;
   bodega.idBodega = indiceBloque;
   memset( bodega.nombre, 0, 28 );
   strncpy( bodega.nombre, nombre.c_str(), 27 );
   memset( bodega.ubicacion, 0, 24 );
   strncpy( bodega.ubicacion, ubicacion.c_str(), 23 );
   bodega.capacidadMaxima = capacidadMaxima;
   bodega.cantidadProductos = 0;

   char buffer[ TAMANO_BLOQUE ];
   bodega.serializar( buffer );
   escribirBloque( indiceBloque, buffer );

   control.cantidadBodegas++;
   guardarControl();

   return bodega.idBodega;

}

std::vector<InfoBodega> FileSystem::listarBodegas() {

   std::vector<InfoBodega> resultado;
   char buffer[ TAMANO_BLOQUE ];

   for ( int32_t i = 1; i <= control.cantidadBodegas; i++ ) {
      leerBloque( i, buffer );
      BloqueBodega bodega;
      bodega.deserializar( buffer );
      resultado.push_back( { bodega.idBodega, bodega.nombre, bodega.ubicacion, bodega.capacidadMaxima } );
   }

   return resultado;

}



bool FileSystem::agregarProducto( int32_t idBodega, const std::string & nombreProducto, int32_t cantidad, std::string * motivoError ) {

   if ( idBodega < 1 || idBodega > control.cantidadBodegas ) {
      if ( motivoError ) *motivoError = "BODEGA_NO_ENCONTRADA";
      return false;
   }

   char buffer[ TAMANO_BLOQUE ];
   leerBloque( idBodega, buffer );
   BloqueBodega bodega;
   bodega.deserializar( buffer );

   if ( bodega.cantidadProductos >= MAX_PRODUCTOS_BODEGA ) {
      if ( motivoError ) *motivoError = "BODEGA_LLENA (maximo " + std::to_string( MAX_PRODUCTOS_BODEGA ) + " productos)";
      return false;
   }

   Producto & nuevo = bodega.productos[ bodega.cantidadProductos ];
   memset( nuevo.nombre, 0, 16 );
   strncpy( nuevo.nombre, nombreProducto.c_str(), 15 );
   nuevo.cantidad = cantidad;
   bodega.cantidadProductos++;

   bodega.serializar( buffer );
   escribirBloque( idBodega, buffer );

   return true;

}

std::vector<Producto> FileSystem::listarProductosDeBodega( int32_t idBodega ) {

   std::vector<Producto> resultado;
   if ( idBodega < 1 || idBodega > control.cantidadBodegas ) return resultado;

   char buffer[ TAMANO_BLOQUE ];
   leerBloque( idBodega, buffer );
   BloqueBodega bodega;
   bodega.deserializar( buffer );

   for ( int i = 0; i < bodega.cantidadProductos; i++ ) {
      resultado.push_back( bodega.productos[ i ] );
   }

   return resultado;

}

std::vector<ProductoConOrigen> FileSystem::buscarProductoEnTodasBodegas( const std::string & nombreProducto ) {

   std::vector<ProductoConOrigen> resultado;
   char buffer[ TAMANO_BLOQUE ];

   for ( int32_t i = 1; i <= control.cantidadBodegas; i++ ) {
      leerBloque( i, buffer );
      BloqueBodega bodega;
      bodega.deserializar( buffer );

      for ( int j = 0; j < bodega.cantidadProductos; j++ ) {
         if ( nombreProducto == std::string( bodega.productos[ j ].nombre ) ) {
            resultado.push_back( { bodega.productos[ j ].nombre, bodega.productos[ j ].cantidad, bodega.idBodega, bodega.nombre } );
         }
      }
   }

   return resultado;

}

bool FileSystem::reservarProducto( int32_t idBodega, const std::string & nombreProducto, int32_t cantidad, std::string * motivoError ) {

   if ( idBodega < 1 || idBodega > control.cantidadBodegas ) {
      if ( motivoError ) *motivoError = "BODEGA_NO_ENCONTRADA";
      return false;
   }

   char buffer[ TAMANO_BLOQUE ];
   leerBloque( idBodega, buffer );
   BloqueBodega bodega;
   bodega.deserializar( buffer );

   for ( int i = 0; i < bodega.cantidadProductos; i++ ) {
      if ( nombreProducto == std::string( bodega.productos[ i ].nombre ) ) {
         if ( bodega.productos[ i ].cantidad < cantidad ) {
            if ( motivoError ) *motivoError = "STOCK_INSUFICIENTE";
            return false;
         }
         bodega.productos[ i ].cantidad -= cantidad;
         bodega.serializar( buffer );
         escribirBloque( idBodega, buffer );
         return true;
      }
   }

   if ( motivoError ) *motivoError = "PRODUCTO_NO_ENCONTRADO";
   return false;

}

void FileSystem::imprimirEstado() {
   std::cout << "Bodegas registradas: " << control.cantidadBodegas << " (bloques 1.." << control.cantidadBodegas << ")\n";
}