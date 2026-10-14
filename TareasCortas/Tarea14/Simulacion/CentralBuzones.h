

#ifndef CENTRALBUZONES_H
#define CENTRALBUZONES_H

// Bibliotecas incluidas
#include "Buzon.h"
#include "Mensaje.h"
#include <unordered_map>
#include <mutex>

// Clase CentralBuzones
class CentralBuzones {
  public:
    // Crea un buzón nuevo para el id indicado, si todavía no existe
    void registrar(int idCliente);
    // Encola un mensaje en el buzón del id indicado
    void encolar(int idCliente, const Mensaje &mensaje);
    // Desencola (bloqueante) el siguiente mensaje del buzón del id indicado
    Mensaje desencolar(int idCliente);

  private:
    // Mapa que asocia cada id de cliente con su buzón; vive mientras dure el programa
    std::unordered_map<int, Buzon> buzones;
    // Candado definido para proteger el mapa de buzones de condiciones de carrera
    std::mutex candado;
};

#endif