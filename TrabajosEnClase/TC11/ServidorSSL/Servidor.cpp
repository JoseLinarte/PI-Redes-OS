#include <iostream>
#include <thread>
#include "Servidor.hpp"
#include "Manejadorconexion.hpp"

// El constructor de Socket ya crea el descriptor (socket() se llama
// dentro de VSocket::Init, invocado desde el constructor de Socket).
// 's' = stream (TCP), false = IPv4.
Servidor::Servidor( int puerto, const char * cert, const char * llave ): puerto( puerto ), escucha( cert, llave, false ), corriendo( false ) {}
bool Servidor::Iniciar() {
   try {
      escucha.Bind( puerto );
      escucha.Listen( 16 );   // backlog de 16 conexiones pendientes
   } catch ( const std::exception & e ) {
      std::cerr << "No se pudo iniciar el servidor: " << e.what() << "\n";
      return false;
   }

   corriendo = true;
   std::cout << "Servidor escuchando en el puerto " << puerto << "\n";
   return true;
}

void Servidor::Ejecutar( ServicioProductos & servicio ) {
   while ( corriendo ) {
      try {
         std::unique_ptr<SSLSocket> cliente(
            static_cast<SSLSocket *>( escucha.AcceptConnection() ) );
         cliente->CopyContext( &escucha );   // comparte el SSL_CTX, crea SSL* propio

         std::thread hilo( &ManejadorConexion::Atender, std::move( cliente ), std::ref( servicio ) );
         hilo.detach();
      } catch ( const std::exception & e ) {
         // Un Accept() fallido (ej. interrumpido por señal) no debe
         // tumbar el servidor completo: se loguea y se sigue esperando.
         std::cerr << "[Servidor] Accept() fallo: " << e.what() << "\n";
      }
   }
}

void Servidor::Detener() {
   corriendo = false;
   escucha.Close();
}