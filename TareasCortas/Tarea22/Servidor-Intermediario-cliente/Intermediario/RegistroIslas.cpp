#include "RegistroIslas.hpp"

InfoIsla * RegistroIslas::Buscar( const std::string & id ) {
   for ( InfoIsla & i : islas ) {
      if ( i.id == id ) return &i;
   }
   return nullptr;
}

bool RegistroIslas::Registrar( const std::string & id, const std::string & ip, int puerto ) {
   std::lock_guard<std::mutex> lock( mtx );
   bool cambio = false;
   InfoIsla * i = Buscar( id );
   if ( i == nullptr ) {
      islas.push_back( InfoIsla() );
      i = &islas.back();
      i->id = id;
      cambio = true;
   } else if ( !i->activa ) {
      cambio = true;
   }
   i->ip = ip;
   i->puerto = puerto;
   i->activa = true;
   i->ultimoContacto = std::time( nullptr );
   return cambio;
}

void RegistroIslas::FijarCategorias( const std::string & id, const std::vector<std::string> & categorias ) {
   std::lock_guard<std::mutex> lock( mtx );
   InfoIsla * i = Buscar( id );
   if ( i != nullptr ) i->categorias = categorias;
}

bool RegistroIslas::MarcarInactiva( const std::string & id ) {
   std::lock_guard<std::mutex> lock( mtx );
   InfoIsla * i = Buscar( id );
   if ( i == nullptr || !i->activa ) return false;
   i->activa = false;
   return true;
}

std::vector<InfoIsla> RegistroIslas::Todas() const {
   std::lock_guard<std::mutex> lock( mtx );
   return islas;
}

std::vector<InfoIsla> RegistroIslas::Activas() const {
   std::lock_guard<std::mutex> lock( mtx );
   std::vector<InfoIsla> r;
   for ( const InfoIsla & i : islas ) {
      if ( i.activa ) r.push_back( i );
   }
   return r;
}
bool RegistroIslas::Obtener( const std::string & id, InfoIsla & resultado ) const {
   std::lock_guard<std::mutex> lock( mtx );
   for ( const InfoIsla & i : islas ) {
      if ( i.id == id ) {
         resultado = i;
         return true;
      }
   }
   return false;
}