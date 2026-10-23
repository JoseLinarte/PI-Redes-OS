#include "RedSimulada.h"

static std::string clave(const std::string &ip, int puerto) {
  return ip + ":" + std::to_string(puerto);
}

void RedSimulada::conectar(IslaSimulada *isla) {
  std::lock_guard<std::mutex> bloquear(this->candado);
  this->islas[clave(isla->ip(), isla->puerto())] = isla;
}

bool RedSimulada::enviarSolicitud(const std::string &ip, int puerto, const std::string &solicitud, std::string &respuesta) {
  IslaSimulada *destino = nullptr;
  {
    std::lock_guard<std::mutex> bloquear(this->candado);
    auto it = this->islas.find(clave(ip, puerto));
    if (it == this->islas.end()) return false;
    destino = it->second;
  }
  respuesta = destino->atender(solicitud);
  return true;
}