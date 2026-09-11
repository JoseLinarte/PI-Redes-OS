/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  *  FileSystem de bodegas - version provisional
  *  Bloques fijos de 256 bytes, sin cadenas de continuacion ni lista de libres.
  *  Bloque 0 = control. Bloques 1..N = una bodega completa cada uno.
  *
 **/

#ifndef FILESYSTEM_BODEGAS_HPP
#define FILESYSTEM_BODEGAS_HPP

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

constexpr int     TAMANO_BLOQUE      = 256;
constexpr int32_t BLOQUE_CONTROL     = 0;
constexpr int     MAX_PRODUCTOS_BODEGA = 9;

struct BloqueControl {
   int32_t cantidadBodegas = 0;

   void serializar( char * buffer ) const;
   void deserializar( const char * buffer );
};

struct Producto {
   char    nombre[ 16 ] = { 0 };
   int32_t cantidad     = 0;
};

struct BloqueBodega {
   int32_t idBodega          = 0;
   char    nombre[ 28 ]      = { 0 };
   char    ubicacion[ 24 ]   = { 0 };
   int32_t capacidadMaxima   = 0;
   int32_t cantidadProductos = 0;
   Producto productos[ MAX_PRODUCTOS_BODEGA ];

   void serializar( char * buffer ) const;
   void deserializar( const char * buffer );
};

struct ProductoConOrigen {
   std::string nombreProducto;
   int32_t     cantidad;
   int32_t     idBodega;
   std::string nombreBodega;
};

struct InfoBodega {
   int32_t     idBodega;
   std::string nombre;
   std::string ubicacion;
   int32_t     capacidadMaxima;
};

class FileSystem {
   public:
      explicit FileSystem( const std::string & archivo, bool * eraNuevo = nullptr );
      ~FileSystem();

      int32_t crearBodega( const std::string & nombre, const std::string & ubicacion, int32_t capacidadMaxima );
      std::vector<InfoBodega> listarBodegas();

      // true si se pudo agregar; false si la bodega ya tiene los 9 productos llenos
      bool agregarProducto( int32_t idBodega, const std::string & nombreProducto, int32_t cantidad, std::string * motivoError = nullptr );
      std::vector<Producto> listarProductosDeBodega( int32_t idBodega );

      std::vector<ProductoConOrigen> buscarProductoEnTodasBodegas( const std::string & nombreProducto );

      bool reservarProducto( int32_t idBodega, const std::string & nombreProducto, int32_t cantidad, std::string * motivoError = nullptr );

      void imprimirEstado();

   private:
      std::fstream  disco;
      BloqueControl control;

      void inicializarArchivo();
      void leerBloque( int32_t numeroBloque, char * buffer );
      void escribirBloque( int32_t numeroBloque, const char * buffer );
      void guardarControl();
};

#endif