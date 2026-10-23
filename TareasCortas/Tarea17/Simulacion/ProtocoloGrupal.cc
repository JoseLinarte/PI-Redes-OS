#include "ProtocoloGrupal.h"
#include <cstdio>
#include <regex>

namespace Grupal {

std::vector<std::string> dividir(const std::string &texto, char separador) {
  std::vector<std::string> campos;
  size_t inicio = 0;
  while (true) {
    size_t fin = texto.find(separador, inicio);
    if (fin == std::string::npos) { campos.push_back(texto.substr(inicio)); break; }
    campos.push_back(texto.substr(inicio, fin - inicio));
    inicio = fin + 1;
  }
  return campos;
}

static bool coincide(const std::string &valor, const char *patron) {
  return std::regex_match(valor, std::regex(patron));
}

bool esTipo(const std::string &v)      { return coincide(v, "^[0-9]{2}$"); }
bool esIdMsg(const std::string &v)     { return coincide(v, "^[0-9]{4}$"); }
bool esIdIsla(const std::string &v)    { return coincide(v, "^INT_[0-9]{2}$"); }
bool esCategoria(const std::string &v) { return coincide(v, "^[a-zA-Z0-9_\\-]{1,50}$"); }
bool esProducto(const std::string &v)  { return coincide(v, "^[a-zA-Z0-9_\\-]{1,50}$"); }
bool esCantidad(const std::string &v)  { return coincide(v, "^[0-9]{1,5}$"); }
bool esCount(const std::string &v)     { return coincide(v, "^[0-9]{1,3}$"); }

bool esIp(const std::string &v) {
  if (!coincide(v, "^([0-9]{1,3}\\.){3}[0-9]{1,3}$")) return false;
  int a, b, c, d;
  if (std::sscanf(v.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4) return false;
  return a <= 255 && b <= 255 && c <= 255 && d <= 255;
}

bool esPuerto(const std::string &v) {
  if (!coincide(v, "^[0-9]{1,5}$")) return false;
  int p = std::stoi(v);
  return p >= 1 && p <= 65535;
}

}