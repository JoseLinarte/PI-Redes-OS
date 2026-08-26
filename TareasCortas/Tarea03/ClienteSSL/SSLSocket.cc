/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *  SSL Socket class implementation
  *
  * (Fedora version)
  *
 **/
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>

#include "SSLSocket.h"
#include "Socket.h"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 ) {

   this->Init( 's', IPv6 );

   this->Context = nullptr;
   this->BIO = nullptr;

   this->InitSSL();					// Initializes to client context

}


/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool IPv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( char * certFileName, char * keyFileName, bool IPv6 ) {
   this->Init( 's', IPv6 );

   this->Context = nullptr;
   this->BIO = nullptr;

   this->InitSSL( true );
   this->LoadCertificates( certFileName, keyFileName );
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->Init( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {
   if ( nullptr != this->BIO ) {
      SSL_shutdown( reinterpret_cast<SSL *>( this->BIO ) );
      SSL_free( reinterpret_cast<SSL *>( this->BIO ) );
      this->BIO = nullptr;
   }
   if ( nullptr != this->Context ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->Context ) );
      this->Context = nullptr;
   }
   this->Close();
}


/**
  *  InitSSL
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::InitSSL( bool serverContext ) {
   this->InitContext( serverContext );

   SSL * ssl = SSL_new( reinterpret_cast<SSL_CTX *>( this->Context ) );
   if ( nullptr == ssl ) {
      throw std::runtime_error( "SSLSocket::InitSSL( bool ) [SSL_new]" );
   }

   this->BIO = (void *) ssl;
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext( bool serverContext ) {
   const SSL_METHOD * method;
   SSL_CTX * context;

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   if ( serverContext ) {
      method = TLS_server_method();
   } else {
      method = TLS_client_method();
   }

   if ( nullptr == method ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

   context = SSL_CTX_new( method );
   if ( nullptr == context ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool ) [SSL_CTX_new]" );
   }

   this->Context = (void *) context;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
 void SSLSocket::LoadCertificates( const char * certFileName, const char * keyFileName ) {
   SSL_CTX * context = reinterpret_cast<SSL_CTX *>( this->Context );

   if ( SSL_CTX_use_certificate_file( context, certFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      throw std::runtime_error( "SSLSocket::LoadCertificates [certificate]" );
   }
   if ( SSL_CTX_use_PrivateKey_file( context, keyFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      throw std::runtime_error( "SSLSocket::LoadCertificates [private key]" );
   }
   if ( !SSL_CTX_check_private_key( context ) ) {
      throw std::runtime_error( "SSLSocket::LoadCertificates [key mismatch]" );
   }
}
 

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::Connect( const char * hostName, int port ) {
   int st;

   st = this->TryToConnect( hostName, port );		
   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Connect( const char *, int ) [TryToConnect]" );
   }

   SSL_set_fd( reinterpret_cast<SSL *>( this->BIO ), this->sockId );

   st = SSL_connect( reinterpret_cast<SSL *>( this->BIO ) );
   if ( 1 != st ) {
      SSL_get_error( reinterpret_cast<SSL *>( this->BIO ), st );
      throw std::runtime_error( "SSLSocket::Connect( const char *, int ) [SSL_connect]" );
   }

   return st;
}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::Connect( const char * host, const char * service ) {
   int st;

   st = this->TryToConnect( host, service );
   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Connect( const char *, const char * ) [TryToConnect]" );
   }

   SSL_set_fd( reinterpret_cast<SSL *>( this->BIO ), this->sockId );

   st = SSL_connect( reinterpret_cast<SSL *>( this->BIO ) );
   if ( 1 != st ) {
      SSL_get_error( reinterpret_cast<SSL *>( this->BIO ), st );
      throw std::runtime_error( "SSLSocket::Connect( const char *, const char * ) [SSL_connect]" );
   }

   return st;
}


/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Read( void * buffer, size_t size ) {
   int st;

   st = SSL_read( reinterpret_cast<SSL *>( this->BIO ), buffer, (int) size );
   if ( st <= 0 ) {
      SSL_get_error( reinterpret_cast<SSL *>( this->BIO ), st );
      throw std::runtime_error( "SSLSocket::Read( void *, size_t )" );
   }

   return (size_t) st;
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write( const char * string ) {
   return this->Write( (const void *) string, strlen( string ) );
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	const void * buffer to store data to write
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {
   int st;

   st = SSL_write( reinterpret_cast<SSL *>( this->BIO ), buffer, (int) size );
   if ( st <= 0 ) {
      SSL_get_error( reinterpret_cast<SSL *>( this->BIO ), st );
      throw std::runtime_error( "SSLSocket::Write( const void *, size_t )" );
   }

   return (size_t) st;
}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( (SSL *) this->BIO );		 // Get certificates (if available)
   if ( nullptr != cert ) {
      printf("Server certificates:\n");
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
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->BIO ) );

}

