#ifndef RegistroIslas_hpp
#define RegistroIslas_hpp

#include <ctime>
#include <mutex>
#include <string>
#include <vector>

struct InfoIsla {
   std::string id;                       // INT_xx
   std::string ip;
   int puerto = 0;
   std::vector<std::string> categorias;
   bool activa = false;
   std::time_t ultimoContacto = 0;
};

// Tabla de rutas hacia otras islas. Segura para varios hilos.
class RegistroIslas {
   public:
      // Crea o actualiza la isla y la marca activa. true si era nueva o estaba inactiva.
      bool Registrar( const std::string & id, const std::string & ip, int puerto );
      void FijarCategorias( const std::string & id, const std::vector<std::string> & categorias );
      // true si estaba activa y paso a inactiva
      bool MarcarInactiva( const std::string & id );
      bool Obtener( const std::string & id, InfoIsla & resultado ) const;
      std::vector<InfoIsla> Todas() const;
      std::vector<InfoIsla> Activas() const;

   private:
      InfoIsla * Buscar( const std::string & id );   // requiere mtx tomado

      mutable std::mutex mtx;
      std::vector<InfoIsla> islas;
};

#endif