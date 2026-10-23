#ifndef PROTOCOLOGRUPAL_H
#define PROTOCOLOGRUPAL_H

#include <string>
#include <vector>

// Utilidades del protocolo mancomunado: división de campos y validación con regex
namespace Grupal {
  // Códigos de error del protocolo (02/código)
  const std::string ERR_MAL_FORMADO = "100";
  const std::string ERR_TIPO_NO_SOPORTADO = "101";
  const std::string ERR_VALOR_INVALIDO = "102";
  const std::string ERR_STOCK_INSUFICIENTE = "103";
  const std::string ERR_SIN_RESERVA = "104";

  // Divide por el separador conservando los campos vacíos
  std::vector<std::string> dividir(const std::string &texto, char separador);

  bool esTipo(const std::string &valor);
  bool esIdMsg(const std::string &valor);
  bool esIdIsla(const std::string &valor);
  bool esIp(const std::string &valor);
  bool esPuerto(const std::string &valor);
  bool esCategoria(const std::string &valor);
  bool esProducto(const std::string &valor);
  bool esCantidad(const std::string &valor);
  bool esCount(const std::string &valor);
}

#endif