#ifndef REDSIMULADA_H
#define REDSIMULADA_H

#include "IslaSimulada.h"
#include <map>
#include <mutex>
#include <string>

class RedSimulada {
  public:
    void conectar(IslaSimulada *isla);
    bool enviarSolicitud(const std::string &ip, int puerto, const std::string &solicitud, std::string &respuesta);

  private:
    std::map<std::string, IslaSimulada *> islas;
    std::mutex candado;
};

#endif