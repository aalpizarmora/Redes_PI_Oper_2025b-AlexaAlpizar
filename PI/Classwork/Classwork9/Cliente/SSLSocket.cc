/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *  Socket class implementation
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
SSLSocket::SSLSocket( bool IPv6 , bool context) {

   this->BuildSocket( 's', IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   this->Init(context);					// Initializes to client context


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
SSLSocket::SSLSocket( const char * certFileName, const char * keyFileName, bool context ) {
    this->BuildSocket('s', false);

    this->SSLContext = nullptr;
    this->SSLStruct = nullptr;

    this->certfilename = certFileName;
    this->keyfilename = keyFileName;
    printf("Cert: %s Key: %s\n", certFileName, keyFileName);

    this->Init(context);  // contexto de servidor
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->BuildSocket( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {

// SSL destroy
   if ( nullptr != this->SSLContext ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->SSLContext ) );
   }
   if ( nullptr != this->SSLStruct ) {
      SSL_free( reinterpret_cast<SSL *>( this->SSLStruct ) );
   }

   this->Close();

}


/**
  *  SSLInit
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::Init(bool context) {
   if (context){
      printf("Client context\n");
      this->InitContext(); // primero contexto de cliente

   }
   else {
      printf("Server context\n");
      this->InitServer();
   }
   
    SSL* ssl = SSL_new((SSL_CTX*)this->SSLContext);
    if (nullptr == ssl) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("SSLSocket::Init - Cannot create SSL object");
    }

    this->SSLStruct = (void*)ssl;
}


void SSLSocket::InitServer(){
    this->InitServerContext();
    SSL_CTX *context = reinterpret_cast<SSL_CTX *>(this->SSLContext);
    SSL *ssl = SSL_new(context); // Crear la estructura SSL
    if (!ssl) {
       throw std::runtime_error("SSLSocket::Init( bool ): Error al crear SSL");
    }
    this->LoadCertificates(); // Cargar certificados
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext() {
   const SSL_METHOD * method;
   SSL_CTX * context;
   method = TLS_client_method(); // inicia conexiones
   context = SSL_CTX_new(method);

   if (nullptr == context) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error("SSLSocket::InitContext- Cannot create");
      }

   this->SSLContext = (void *) context;
}

void SSLSocket::InitServerContext() {
    const SSL_METHOD* method;
    SSL_CTX* context;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    method = TLS_server_method();  // servidor espera conexiones
    if (nullptr == method) {
        throw std::runtime_error("SSLSocket::InitServerContext - No SSL_METHOD available");
    }

    context = SSL_CTX_new(method);
    if (nullptr == context) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("SSLSocket::InitServerContext - Cannot create context");
    }

    this->SSLContext = (void*)context;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
 void SSLSocket::LoadCertificates() {
       // Cargar certificado y llave
    SSL_CTX* ctx = (SSL_CTX*)this->SSLContext;

    if (SSL_CTX_use_certificate_file(ctx, this->certfilename, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Failed to load certificate");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, this->keyfilename, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Failed to load private key");
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        throw std::runtime_error("Private key does not match the certificate public key");
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
int SSLSocket::MakeConnection( const char * hostName, int port ) {
   int st;
   st = this->EstablishConnection(hostName, port);
   SSL *ssl = (SSL *)(this->SSLStruct);

   if (!ssl) {
        throw std::runtime_error("SSLSocket::MakeConnection - SSL object is null");
    }
   
   SSL_set_fd(ssl, this->idSocket);
   if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("SSLSocket::MakeConnection - SSL_connect failed");
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
int SSLSocket::MakeConnection( const char * host, const char * service ) {
   int st;
   st = this->EstablishConnection(host, service);

   SSL *ssl = (SSL *)(this->SSLStruct);
   if (!ssl) {
      throw std::runtime_error("SSLSocket::MakeConnection(service) - SSL object is null");
   }
   
   SSL_set_fd(ssl, this->idSocket);
   if (SSL_connect(ssl) <= 0) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error("SSLSocket::MakeConnection(service) - SSL_connect failed");
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
   size_t st = -1;
   SSL *ssl = (SSL *)(this->SSLStruct);
   st = SSL_read(ssl, buffer, static_cast<int>(size));

   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Read( void *, size_t )" );
   }

   return st;

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

   return this->Write(string, strlen(string));

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
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {
   int st = -1;
   st = SSL_write((SSL*)(this->SSLStruct), buffer, static_cast<int>(size));
   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Write( void *, size_t )" );
   }

   return st;

}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( (SSL *) this->SSLStruct );		 // Get certificates (if available)
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
  * AcceptiConnection method
  *    use base class to accept connections
  *
  *  @returns   a new class instance
  *
  *  Waits for a new connection to service (TCP mode: stream)
  *
 **/
SSLSocket * SSLSocket::AcceptConnection(){
   int id;
   SSLSocket * peer;

   id = this->WaitForConnection();

   peer = new SSLSocket( id );

   return peer;

}

/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->SSLStruct ) );

}

void SSLSocket::CopyContext(SSLSocket* sockerCopy){
   if (!sockerCopy) {
      throw std::invalid_argument("Original SSLSocket is null");
   }

   SSL_CTX *context = reinterpret_cast<SSL_CTX *>(sockerCopy->SSLContext);

   SSL *ssl = SSL_new(context); // Crear la estructura SSL
   if (!ssl) {
       ERR_print_errors_fp(stderr);
       throw std::runtime_error("SSLSocket::Copy: Error al crear SSL");
   }

   this->SSLStruct = ssl;
   if (SSL_set_fd((SSL*)this->SSLStruct, this->idSocket) != 1) {
       ERR_print_errors_fp(stderr);
       throw std::runtime_error("SSLSocket::Copy: Error al asociar socket con SSL");
   }

}

/**
 * Hace el handshake SSL con el cliente
 */
SSLSocket* SSLSocket::Accept() {
    if (!this->SSLStruct) {
        throw std::runtime_error("SSL structure not initialized. Did you call Copy()?");
    }

    if (SSL_accept((SSL*)this->SSLStruct) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("SSL handshake failed");
    }

    return this;
}