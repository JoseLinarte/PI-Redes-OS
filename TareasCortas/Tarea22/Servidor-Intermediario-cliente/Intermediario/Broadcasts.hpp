#ifndef Broadcasts_hpp
#define Broadcasts_hpp

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

inline bool EmpiezaCon( const std::string & s, const std::string & prefijo ) {
   return s.compare( 0, prefijo.size(), prefijo ) == 0;
}

// Broadcast de cada interfaz IPv4 activa que este en 172.16.123.x, mas el de la Isla 3.
inline std::vector<std::string> DetectarBroadcasts() {
   std::vector<std::string> r;
   struct ifaddrs * lista = nullptr;
   if ( getifaddrs( &lista ) == 0 ) {
      for ( struct ifaddrs * a = lista; a != nullptr; a = a->ifa_next ) {
         if ( !a->ifa_addr || a->ifa_addr->sa_family != AF_INET ) continue;
         if ( !( a->ifa_flags & IFF_UP ) || !( a->ifa_flags & IFF_BROADCAST ) ) continue;
         if ( !a->ifa_broadaddr ) continue;
         char ip[INET_ADDRSTRLEN];
         inet_ntop( AF_INET, &( (struct sockaddr_in *)a->ifa_broadaddr )->sin_addr, ip, sizeof( ip ) );
         if ( !EmpiezaCon( ip, "172.16.123." ) ) continue;
         if ( std::find( r.begin(), r.end(), ip ) == r.end() ) r.push_back( ip );
      }
      freeifaddrs( lista );
   }
   if ( std::find( r.begin(), r.end(), "172.16.123.63" ) == r.end() ) r.push_back( "172.16.123.63" );
   return r;
}

inline std::string MiIpPrivada() {
   std::string r;
   struct ifaddrs * lista = nullptr;
   if ( getifaddrs( &lista ) == 0 ) {
      for ( struct ifaddrs * a = lista; a != nullptr; a = a->ifa_next ) {
         if ( !a->ifa_addr || a->ifa_addr->sa_family != AF_INET ) continue;
         if ( !( a->ifa_flags & IFF_UP ) ) continue;
         char ip[INET_ADDRSTRLEN];
         inet_ntop( AF_INET, &( (struct sockaddr_in *)a->ifa_addr )->sin_addr, ip, sizeof( ip ) );
         if ( EmpiezaCon( ip, "172.16.123." ) ) { r = ip; break; }
      }
      freeifaddrs( lista );
   }
   return r;
}

inline std::vector<std::string> BroadcastsOtrasIslas( int miIsla ) {
   static const char * BC[7] = {
      "172.16.123.15", "172.16.123.31", "172.16.123.47", "172.16.123.63",
      "172.16.123.79", "172.16.123.95", "172.16.123.111"
   };
   std::vector<std::string> r;
   for ( int i = 0; i < 7; i++ ) {
      if ( i != miIsla ) r.push_back( BC[i] );
   }
   return r;
}

inline std::vector<std::string> ResolverBroadcasts( const std::string & arg ) {
   if ( arg.empty() || arg == "auto" ) return DetectarBroadcasts();
   std::vector<std::string> r;
   std::stringstream ss( arg );
   std::string x;
   while ( std::getline( ss, x, ',' ) ) {
      if ( !x.empty() ) r.push_back( x );
   }
   return r;
}

inline std::string MiIpPublica() {
   std::string r;
   struct ifaddrs * lista = nullptr;
   if ( getifaddrs( &lista ) == 0 ) {
      for ( struct ifaddrs * a = lista; a != nullptr; a = a->ifa_next ) {
         if ( !a->ifa_addr || a->ifa_addr->sa_family != AF_INET ) continue;
         if ( !( a->ifa_flags & IFF_UP ) ) continue;
         char ip[INET_ADDRSTRLEN];
         inet_ntop( AF_INET, &( (struct sockaddr_in *)a->ifa_addr )->sin_addr, ip, sizeof( ip ) );
         if ( EmpiezaCon( ip, "10.1.35." ) ) {
            r = ip;
            break;
         }
      }
      freeifaddrs( lista );
   }
   return r;
}

#endif