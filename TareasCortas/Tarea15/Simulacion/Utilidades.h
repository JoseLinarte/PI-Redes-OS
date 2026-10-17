
#ifndef UTILIDADES_H
#define UTILIDADES_H

// Bibliotecas incluídas
#include <sstream>
#include <string>
#include <vector>
#include <cstddef>

// Método que separa una cadena en varios elementos utilizando un separador definido
std::vector<std::string> separarCampos(const std::string &texto, char separador);
// Une los campos de un texto separado por un carácter, usando espacios en su lugar
std::string reemplazarSeparador(const std::string &texto, char separadorOriginal);

// Convierte un texto a minúsculas
std::string aMinusculas(std::string texto);
// Copia un texto a un búfer fijo sin desbordarlo (trunca si no cabe)
void copiarContenido(char *destino, size_t tamanio, const std::string &texto);

#endif
