/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  ****** SSLSocket class interface
  *
 **/

#ifndef SSLSocket_h
#define SSLSocket_h

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "Socket.hpp"  
class SSLSocket : public Socket {

   public:
      SSLSocket( bool IPv6 = false );                                              // constructor cliente
      SSLSocket( const char * certFileName, const char * keyFileName, bool IPv6 = false ); // constructor servidor
      SSLSocket( int id );                                                          // envuelve un descriptor ya existente (usado por AcceptConnection)
      ~SSLSocket();

      int Connect( const char *, int );            // TCP connect + SSL_connect
      int Connect( const char *, const char * );

      size_t Read( void *, size_t );                // SSL_read
      size_t Write( const void *, size_t );          // SSL_write
      size_t Write( const char * );

      VSocket * AcceptConnection();                  // accept TCP normal, envuelve en un SSLSocket nuevo

      SSLSocket * Accept();                           // SSL_accept: hace el handshake sobre la conexion ya aceptada
      void CopyContext( SSLSocket * original );       // comparte el SSL_CTX de "original" (usado del lado servidor)
      void ShowCerts();
      const char * GetCipher();

   protected:
      void InitContext();                                            // contexto cliente (TLS_client_method)
      void Init();                                                   // crea el SSL* a partir del contexto ya armado
      void InitServerContext();                                      // contexto servidor (TLS_server_method)
      void InitServer( const char * certFileName, const char * keyFileName );  // contexto servidor + carga certificados
      void LoadCertificates( const char * certFileName, const char * keyFileName );

      SSL_CTX * context;
      SSL     * ssl;
      bool      ownsContext;      // true si este objeto creo el contexto (y debe liberarlo al destruirse)

};

#endif