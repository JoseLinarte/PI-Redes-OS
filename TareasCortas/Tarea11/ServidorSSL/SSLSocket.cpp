/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  ****** SSLSocket class implementation
  *
 **/

#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <sys/socket.h>
#include "SSLSocket.h"


/**
  *  Constructor cliente
  *     crea el socket TCP normal (via Socket), arma el contexto cliente
  *     y el objeto SSL asociado a ese socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 ) : Socket( 's', IPv6 ) {

   this->context     = nullptr;
   this->ssl         = nullptr;
   this->ownsContext = false;

   this->InitContext();     // crea this->context (contexto cliente)
   this->Init();            // crea this->ssl a partir del contexto, lo asocia a sockId

}


/**
  *  Constructor servidor
  *     crea el socket TCP normal, arma el contexto servidor y carga
  *     los certificados. Este objeto NO tiene un SSL* propio (nunca hace
  *     handshake el mismo, solo acepta conexiones nuevas)
  *
 **/
SSLSocket::SSLSocket( const char * certFileName, const char * keyFileName, bool IPv6 ) : Socket( 's', IPv6 ) {

   this->context     = nullptr;
   this->ssl         = nullptr;
   this->ownsContext = false;

   this->InitServer( certFileName, keyFileName );

}


/**
  *  Constructor alterno
  *     envuelve un descriptor ya existente (usado por AcceptConnection).
  *     Todavia no tiene contexto ni SSL*, eso se asigna despues con CopyContext
  *
 **/
SSLSocket::SSLSocket( int id ) : Socket( id, false ) {

   this->context     = nullptr;
   this->ssl         = nullptr;
   this->ownsContext = false;

}


/**
  *  Destructor
  *     libera el SSL*, y el contexto solo si este objeto es su dueno
  *     (evita doble free cuando varios SSLSocket comparten el mismo contexto)
  *
 **/
SSLSocket::~SSLSocket() {

   if ( nullptr != this->ssl ) {
      SSL_shutdown( this->ssl );
      SSL_free( this->ssl );
      this->ssl = nullptr;
   }
}

/**
  * InitContext
  *   crea un contexto SSL para conexiones de CLIENTE
  *
 **/
void SSLSocket::InitContext() {

   const SSL_METHOD * method;

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   method = TLS_client_method();
   this->context = SSL_CTX_new( method );

   if ( nullptr == this->context ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::InitContext, SSL_CTX_new" );
   }

   this->ownsContext = true;

}


/**
  * Init
  *   crea el objeto SSL a partir del contexto ya armado, y lo asocia
  *   al descriptor de este socket (sockId, heredado de VSocket)
  *
 **/
void SSLSocket::Init() {

   this->ssl = SSL_new( this->context );

   if ( nullptr == this->ssl ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Init, SSL_new" );
   }

   SSL_set_fd( this->ssl, this->sockId );

}


/**
  * InitServerContext
  *   crea un contexto SSL para conexiones de SERVIDOR
  *
 **/
void SSLSocket::InitServerContext() {

   const SSL_METHOD * method;

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   method = TLS_server_method();
   this->context = SSL_CTX_new( method );

   if ( nullptr == this->context ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::InitServerContext, SSL_CTX_new" );
   }

   this->ownsContext = true;

}


/**
  * InitServer
  *   arma el contexto de servidor y carga certificado + llave privada
  *
  * @param   const char * certFileName: archivo .pem con el certificado
  * @param   const char * keyFileName: archivo .pem con la llave privada
  *
 **/
void SSLSocket::InitServer( const char * certFileName, const char * keyFileName ) {

   this->InitServerContext();
   this->LoadCertificates( certFileName, keyFileName );

}


/**
  * LoadCertificates
  *   verifica y carga el certificado y la llave privada en el contexto
  *
 **/
void SSLSocket::LoadCertificates( const char * certFileName, const char * keyFileName ) {

   if ( SSL_CTX_use_certificate_file( this->context, certFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::LoadCertificates, use_certificate_file" );
   }

   if ( SSL_CTX_use_PrivateKey_file( this->context, keyFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::LoadCertificates, use_PrivateKey_file" );
   }

   if ( !SSL_CTX_check_private_key( this->context ) ) {
      throw std::runtime_error( "SSLSocket::LoadCertificates, la llave privada no coincide con el certificado" );
   }

}


/**
  * Connect (por IP)
  *   hace el connect TCP normal (heredado de Socket) y despues el
  *   handshake SSL sobre esa misma conexion
  *
 **/
int SSLSocket::Connect( const char * hostip, int port ) {

   int st = Socket::Connect( hostip, port );      // TCP connect normal

   int sslSt = SSL_connect( this->ssl );
   if ( 1 != sslSt ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Connect, SSL_connect" );
   }

   return st;

}


/**
  * Connect (por DNS/servicio)
  *
 **/
int SSLSocket::Connect( const char * host, const char * service ) {

   int st = Socket::Connect( host, service );

   int sslSt = SSL_connect( this->ssl );
   if ( 1 != sslSt ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Connect, SSL_connect" );
   }

   return st;

}


/**
  * Read
  *   usa SSL_read en vez del "read" de Unix
  *
 **/
size_t SSLSocket::Read( void * buffer, size_t size ) {

   int st = SSL_read( this->ssl, buffer, (int) size );

   if ( st < 0 ) {                       
      throw std::runtime_error( "SSLSocket::Read, SSL_read" );
   }

   return (size_t) st;

}


/**
  * Write
  *   usa SSL_write en vez del "write" de Unix
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {

   int st = SSL_write( this->ssl, buffer, (int) size );

   if ( st <= 0 ) {
      throw std::runtime_error( "SSLSocket::Write, SSL_write" );
   }

   return (size_t) st;

}

size_t SSLSocket::Write( const char * text ) {
   return this->Write( (const void *) text, strlen( text ) );
}


/**
  * AcceptConnection
  *   acepta una conexion TCP normal (usa WaitForConnection, heredado
  *   de VSocket) y la envuelve en un SSLSocket nuevo, todavia sin
  *   contexto/SSL asignado (eso se hace despues con CopyContext)
  *
 **/
VSocket * SSLSocket::AcceptConnection() {

   int newSockId;
   struct sockaddr_storage clientAddr;
   socklen_t len = sizeof( clientAddr );

   newSockId = accept( this->sockId, (struct sockaddr *) &clientAddr, &len );
   if ( -1 == newSockId ) {
      throw std::runtime_error( "SSLSocket::AcceptConnection, accept" );
   }

   return new SSLSocket( newSockId );

}


/**
  * CopyContext
  *   comparte el contexto SSL de "original" (el socket servidor
  *   principal, que ya tiene los certificados cargados), crea un
  *   SSL* nuevo para ESTA conexion en particular, y lo asocia a su
  *   propio descriptor
  *
 **/
void SSLSocket::CopyContext( SSLSocket * original ) {

   this->context     = original->context;   // comparte el mismo SSL_CTX (no lo crea, no lo libera)
   this->ownsContext = false;

   this->ssl = SSL_new( this->context );
   if ( nullptr == this->ssl ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::CopyContext, SSL_new" );
   }

   SSL_set_fd( this->ssl, this->sockId );

}


/**
  * Accept
  *   realiza el handshake SSL/TLS sobre la conexion ya aceptada
  *   (llamar despues de CopyContext)
  *
 **/
SSLSocket * SSLSocket::Accept() {

   int st = SSL_accept( this->ssl );
   if ( 1 != st ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Accept, SSL_accept" );
   }

   return this;

}


/**
  * ShowCerts
  *   imprime el certificado del otro extremo de la conexion, si existe
  *
 **/
void SSLSocket::ShowCerts() {

   X509 * cert;
   char * line;

   cert = SSL_get_peer_certificate( this->ssl );

   if ( nullptr != cert ) {
      printf( "Server certificates:\n" );
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
  * GetCipher
  *   devuelve el nombre del cifrado negociado en el handshake
  *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( this->ssl );

}