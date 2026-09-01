#ifndef Producto_h
#define Producto_h

#include <string>
#include <vector>

struct Producto {
   std::string intermediario;
   std::string bodega;
   std::string categoria;
   std::string descripcion;
   int cantidad;
   double precio;
};

std::vector<Producto> parsearProductos( const std::string & body );

#endif