#include <cctype>
#include <exception>
#include <sstream>
#include "ProtocoloIslas.hpp"
#include "Socket.hpp"
#include <cstdlib>
#include <cstdio>

std::vector<std::string> ProtocoloIslas::Partir( const std::string & t, char sep ) {
   std::vector<std::string> r;
   std::stringstream ss( t );
   std::string x;
   while ( std::getline( ss, x, sep ) ) r.push_back( x );
   return r;
}

bool ProtocoloIslas::CategoriaValida( const std::string & c ) {
   if ( c.empty() || c.size() > 50 ) return false;
   for ( unsigned char ch : c ) {
      if ( !( std::isalnum( ch ) || ch == '_' || ch == '-' ) ) return false;
   }
   return true;
}

bool ProtocoloIslas::Enviar( const std::string & ip, int puerto, const std::string & mensaje,std::string & respuesta, int timeoutSeg ) {
   respuesta.clear();
   try {
      Socket s( 's', false );
      s.SetReadTimeout( timeoutSeg );
      s.Connect( ip.c_str(), puerto );
      s.Write( mensaje.data(), mensaje.size() );
      s.CerrarEscritura();

      char buf[2048];
      while ( respuesta.size() < 1000000 ) {
         size_t n = 0;
         try {
            n = s.Read( buf, sizeof( buf ) );
         } catch ( const std::exception & ) {
            break;   
         }
         if ( n == 0 ) break;
         respuesta.append( buf, n );
      }
      return !respuesta.empty();
   } catch ( const std::exception & ) {
      return false;
   }
}

bool ProtocoloIslas::ParsearCategorias( const std::string & respuesta, std::vector<std::string> & categorias ) {
   categorias.clear();
   std::string r = respuesta;
   while ( !r.empty() && ( r.back() == '\n' || r.back() == '\r' ) ) r.pop_back();
   if ( r == "01" ) return true;                       // ACK sin categorias
   if ( r.compare( 0, 3, "01/" ) != 0 ) return false;  // 02 o 03 u otra cosa
   for ( const std::string & c : Partir( r.substr( 3 ), ',' ) ) {
      if ( CategoriaValida( c ) ) categorias.push_back( c );
   }
   return true;
}

std::string ProtocoloIslas::Minusculas( const std::string & s ) {
   std::string r = s;
   for ( char & c : r ) c = (char)std::tolower( (unsigned char)c );
   return r;
}

std::string ProtocoloIslas::ANombre( const std::string & s ) {
   std::string r;
   for ( unsigned char c : s ) {
      char x = ( c < 128 ) ? (char)std::tolower( c ) : '_';
      if ( !( std::isalnum( (unsigned char)x ) || x == '-' || x == '_' ) ) x = '_';
      if ( x == '_' && !r.empty() && r.back() == '_' ) continue;
      r += x;
   }
   while ( !r.empty() && r.back() == '_' ) r.pop_back();
   if ( r.size() > 50 ) r.resize( 50 );
   return r;
}

std::string ProtocoloIslas::Precio( const std::string & p ) {
   char buf[32];
   std::snprintf( buf, sizeof( buf ), "%.2f", std::atof( p.c_str() ) );
   return buf;
}

bool ProtocoloIslas::SoloDigitos( const std::string & s, size_t min, size_t max ) {
   if ( s.size() < min || s.size() > max ) return false;
   for ( unsigned char c : s ) {
      if ( !std::isdigit( c ) ) return false;
   }
   return true;
}