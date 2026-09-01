#ifndef Carrito_hpp
#define Carrito_hpp

#include <vector>
#include "Producto.hpp"

struct ItemCarrito {
   Producto producto;
   int cantidadElegida;
};

class Carrito {
   public:
      void Agregar( const Producto & p, int cantidad );
      double Total() const;
      void MostrarFactura() const;

   private:
      std::vector<ItemCarrito> items;
};

#endif