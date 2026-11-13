#ifndef ProtocoloIslas_hpp
#define ProtocoloIslas_hpp

#include <string>
#include <vector>

class ProtocoloIslas {
   public:
      static bool Enviar( const std::string & ip, int puerto, const std::string & mensaje,std::string & respuesta, int timeoutSeg = 5 );

      static bool ParsearCategorias( const std::string & respuesta, std::vector<std::string> & categorias );

      static bool CategoriaValida( const std::string & c );
      static std::vector<std::string> Partir( const std::string & t, char sep );
      static std::string Minusculas( const std::string & s );
      static std::string ANombre( const std::string & s );
      static std::string Precio( const std::string & p );
      static bool SoloDigitos( const std::string & s, size_t min, size_t max );
};

#endif