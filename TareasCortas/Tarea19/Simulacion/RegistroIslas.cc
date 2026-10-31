#include "RegistroIslas.h"
#include "Mensaje.h"
#include "ProtocoloGrupal.h"
#include <algorithm>

static const size_t MAX_VISTOS = 200;

std::string RegistroIslas::procesarAnuncio(const std::string &mensaje) {
  std::lock_guard<std::mutex> bloquear(this->candado);
  std::vector<std::string> campos = Grupal::dividir(mensaje, '/');
  if (!Grupal::esTipo(campos[0])) return "";

  if (campos[0] == "60" && campos.size() == 5) {
    const std::string &idMsg = campos[1], &idIsla = campos[2], &ip = campos[3], &puerto = campos[4];
    if (!Grupal::esIdMsg(idMsg) || !Grupal::esIdIsla(idIsla) || !Grupal::esIp(ip) || !Grupal::esPuerto(puerto)) {
      return "";
    }
    // Eco de nuestro propio anuncio
    if (idIsla == ID_ISLA) return "";
    // Duplicado
    std::string clave = idIsla + "/" + idMsg;
    if (std::find(this->vistos.begin(), this->vistos.end(), clave) != this->vistos.end()) return "";
    this->vistos.push_back(clave);
    if (this->vistos.size() > MAX_VISTOS) this->vistos.pop_front();

    this->islas[idIsla] = InfoIsla{idIsla, ip, std::stoi(puerto)};
    return "Isla registrada: " + idIsla;
  }
  if (campos[0] == "61" && campos.size() == 2) {
    if (!Grupal::esIdIsla(campos[1]) || campos[1] == ID_ISLA) return "";
    if (this->islas.erase(campos[1]) == 0) return "";
    return "Isla eliminada: " + campos[1];
  }
  return "";
}

bool RegistroIslas::obtener(const std::string &id, InfoIsla &resultado) const {
  std::lock_guard<std::mutex> bloquear(this->candado);
  auto it = this->islas.find(id);
  if (it == this->islas.end()) return false;
  resultado = it->second;
  return true;
}

std::vector<InfoIsla> RegistroIslas::listar() const {
  std::lock_guard<std::mutex> bloquear(this->candado);
  std::vector<InfoIsla> lista;
  for (const auto &par : this->islas) lista.push_back(par.second);
  return lista;
}