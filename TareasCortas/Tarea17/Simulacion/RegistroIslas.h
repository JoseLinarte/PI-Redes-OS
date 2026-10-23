#ifndef REGISTROISLAS_H
#define REGISTROISLAS_H

#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Datos de contacto de una isla conocida
struct InfoIsla {
  std::string id;
  std::string ip;
  int puerto;
};

class RegistroIslas {
  public:
    std::string procesarAnuncio(const std::string &mensaje);
    // Busca una isla por id; retorna false si no se conoce
    bool obtener(const std::string &id, InfoIsla &resultado) const;
    // Retorna una copia de todas las islas conocidas
    std::vector<InfoIsla> listar() const;

  private:
    mutable std::mutex candado;
    std::map<std::string, InfoIsla> islas;
    std::deque<std::string> vistos;
};

#endif