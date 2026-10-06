#ifndef ManejadorIntermediario_hpp
#define ManejadorIntermediario_hpp

#include <string>
#include "Socket.hpp"

// Atiende una conexion de cliente lee el request completo y lo reenvia al servidor de productos, y devuelve la respuesta tal cual al cliente
class ManejadorIntermediario {
   public:
      static void Atender( Socket cliente, std::string hostProductos, std::string puertoProductos );

   private:
      static std::string LeerRequestCompleto( Socket & cliente );
      static std::string ReenviarAProductos( const std::string & request,const std::string & host,const std::string & puerto );
      static void EscribirCompleto( Socket & s, const std::string & datos );
      static std::string RespuestaError( int codigo, const std::string & frase, const std::string & detalle );

      static const size_t TAMANO_BUFFER = 4096;
      static const size_t LIMITE_REQUEST = 1024 * 1024;
      static const int TIMEOUT_SEGUNDOS = 10;
};

#endif