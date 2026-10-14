
#ifndef INTERMEDIARIO_H
#define INTERMEDIARIO_H

// Bibliotecas incluidas
#include "Bitacora.h"
#include "Buzon.h"
#include "CentralBuzones.h"
#include "Mensaje.h"
#include "Utilidades.h"
#include <cstring>
#include <thread>
#include <atomic>
#include <sstream>
#include "RegistroIslas.h"
#include "RedSimulada.h"

// Clase Intermediario
class Intermediario {
  public:
    // Método constructor de la clase Intermediario
    Intermediario(Buzon &, Buzon &, Buzon &, CentralBuzones &, RegistroIslas &, RedSimulada &, Bitacora &);
    // Método destructor de la clase Intermediario
    ~Intermediario();
    // Método que permite iniciar el hilo del intermediario
    void iniciar();
    // Método que permite detener el hilo del intermediario
    void detener();

  private:
    // Buzón en el que se encuentran las solicitudes de cualquier cliente
    Buzon &buzonDesdeCliente;
    // Buzón en el que se encuentran las solicitudes para el servidor
    Buzon &buzonHaciaServidor;
    // Buzón en el que se encuentran las respuestas recibidas desde el servidor
    Buzon &buzonDesdeServidor;
    // Central que direcciona las respuestas al buzón del cliente correspondiente
    CentralBuzones &centralBuzones;
    RegistroIslas &registroIslas;
    // Red simulada por la que se envían las solicitudes a otras islas
    RedSimulada &red;   
    // Referencia a la bitácora del proyecto

    Bitacora &bitacora;
    // Variable atómica definida para almacenar el estado actual del intermediario
    std::atomic<Estado> estado;
    // Hilo definido para ejecutar el intermediario
    std::thread hilo;
    // Registro de las islas descubiertas por ANNOUNCE
    

    // Método que permite procesar la solicitud enviada por el servidor
    Mensaje procesarSolicitud(const Mensaje &);
    // Método que ejecuta el hilo del intermediario
    void ejecutar();
    // Indica si el comando debe resolverse contra otra isla en vez de contra el Servidor local
    bool esComandoIsla(Comando);
    // Convierte la solicitud interna en un mensaje del protocolo mancomunado, lo envía y arma la respuesta para el cliente
    Mensaje atenderIsla(const Mensaje &);
};

#endif
