#include <iostream>
#include <stdexcept>
#include "Cliente.hpp"
#include "Producto.hpp"
#include "Carrito.hpp"

int main() {
   Cliente cliente;
   Carrito carrito;
   bool seguirComprando = true;

   while ( seguirComprando ) {
      std::string categoria;
      std::cout << "Ingrese una categoria (ejemplo: Alimentos y bebidas): ";
      std::getline(std::cin, categoria);

      std::string path = "/TicAmazon/list.php?category=" + UrlEncode(categoria);

      try {
         std::string body = cliente.Get("os.ecci.ucr.ac.cr", "http", path);
         std::vector<Producto> productos = parsearProductos(body);

         if ( productos.empty() ) {
            std::cout << "No se encontraron productos en esa categoria.\n";
         } else {
            for ( size_t i = 0; i < productos.size(); i++ ) {
               printf("[%zu] %s - stock: %d - precio: %.2f (bodega: %s)\n",i, productos[i].descripcion.c_str(),productos[i].cantidad, productos[i].precio,productos[i].bodega.c_str());
            }

            int indice, cantidad;
            std::cout << "Elija un indice de producto (-1 para ninguno): ";
            std::cin >> indice;

            if ( indice >= 0 && indice < (int)productos.size() ) {
               std::cout << "Cantidad deseada: ";
               std::cin >> cantidad;

               if ( cantidad > productos[indice].cantidad ) {
                  std::cout << "No hay suficientes producots. Solo hay: "
                             << productos[indice].cantidad << "\n";
               } else {
                  carrito.Agregar(productos[indice], cantidad);
                  std::cout << "Producto agregado al carrito.\n";
               }
            }
            std::cin.ignore();
         }

      } catch ( const std::runtime_error & e ) {
         std::cout << "Error al consultar el servidor: " << e.what() << "\n";
         std::cout << "Puede intentar con otra categoria.\n";
      }

      std::cout << "Quiere buscar otra categoria? (s/n): ";
      char resp;
      std::cin >> resp;
      std::cin.ignore();
      seguirComprando = (resp == 's' || resp == 'S');
   }

   carrito.MostrarFactura();

   return 0;
}