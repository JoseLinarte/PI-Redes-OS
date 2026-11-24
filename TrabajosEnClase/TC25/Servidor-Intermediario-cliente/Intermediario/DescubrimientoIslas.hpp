#ifndef DescubrimientoIslas_hpp
#define DescubrimientoIslas_hpp

#include <atomic>
#include <deque>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include "Socket.hpp"
#include "ServicioProductosDistribuido.hpp"


class DescubrimientoIslas {
   public:
      DescubrimientoIslas( ServicioProductosDistribuido & servicio, const std::string & miId,
                           const std::string & miIp, int puerto, const std::vector<std::string> & destinos );
      ~DescubrimientoIslas();

      bool Iniciar( int segundos );
      void Detener();    // manda el 61 y termina los hilos

   private:
      void HiloEscucha();
      void HiloAnuncio( int segundos );
      void Anunciar();
      bool EsDuplicado( const std::string & clave );

      ServicioProductosDistribuido & servicio;
      std::string miId;
      std::string miIp;
      int puerto;
      std::vector<std::string> destinos;

      Socket udp;
      std::atomic<bool> corriendo;
      std::thread hiloEscucha;
      std::thread hiloAnuncio;
      int contador;

      std::deque<std::string> vistosOrden;   // solo lo usa el hilo de escucha
      std::set<std::string> vistos;
};

#endif