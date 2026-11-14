#ifndef ServidorIslas_hpp
#define ServidorIslas_hpp

#include "Socket.hpp"
#include "ServicioProductosDistribuido.hpp"

class ServidorIslas {
   public:
      ServidorIslas( ServicioProductosDistribuido & servicio, int puerto, const std::string & ip );
      bool Iniciar();

   private:
      void Aceptar();
      void Atender( Socket & conexion );

      ServicioProductosDistribuido & servicio;
      int puerto;
      std::string ip;
      Socket escucha;
};

#endif