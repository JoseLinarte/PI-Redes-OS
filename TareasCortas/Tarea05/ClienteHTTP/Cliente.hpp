#ifndef Cliente_hpp
#define Cliente_hpp

#include <string>
#include "Socket.h"
#include <sstream>
#include <iomanip>
#include <ios>


inline std::string UrlEncode( const std::string & texto ) {
   std::ostringstream encoded;
   for ( unsigned char c : texto ) {
      if ( isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' ) {
         encoded << c;
      } else {
         encoded << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
      }
   }
   return encoded.str();
}


class Cliente {
   public:
        std::string Get( const char * host, const char * servicio, const std::string & path );
        
   private:
      std::string leerRespuestaCompleta( Socket & s );
      std::string extraerBody( const std::string & respuestaCompleta );
      int extraerContentLength( const std::string & headers );
      int extraerCodigoEstado( const std::string & headers );
};



#endif