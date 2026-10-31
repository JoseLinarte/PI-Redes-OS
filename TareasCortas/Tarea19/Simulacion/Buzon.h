

#ifndef BUZON_H
#define BUZON_H

// Bibliotecas incluidas
#include "Mensaje.h"
#include <queue>
#include <mutex>
#include <condition_variable>

// Clase Buzon
class Buzon {
  public:
    // Guarda un mensaje entrante dentro del buzón y lo notifica al hilo consumidor
    void encolar(const Mensaje &msg);
    // Bloquea al hilo consumidor hasta que haya un mensaje disponible.
    Mensaje desencolar();

  private:
    // Cola definida para almacenar los mensajes entrantes
    std::queue<Mensaje> bufer;
    // Candado definido para proteger el buzón de condiciones de carrera
    std::mutex candado;
    // Señal definida para notificar al hilo consumidor que hay un mensaje disponible
    std::condition_variable hayElementos;
};

#endif
