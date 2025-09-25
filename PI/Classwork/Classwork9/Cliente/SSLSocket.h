/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *   SSL Socket class interface
  *
  * (Fedora version)
  *
 **/

#ifndef SSLSocket_h
#define SSLSocket_h

#include <cstddef>

#include "VSocket.h"


class SSLSocket : public VSocket {

   public:
      SSLSocket( bool IPv6 = false, bool context = true);				// Not possible to create with UDP, client constructor
      SSLSocket( const char * certFileName, const char * keyFileName, bool context );		// For server connections
      SSLSocket( int );
      ~SSLSocket();
      int MakeConnection( const char *, int );
      int MakeConnection( const char *, const char * );
      size_t Write( const char * );
      size_t Write( const void *, size_t );
      size_t Read( void *, size_t );
      void ShowCerts();
      const char * GetCipher();
      SSLSocket * Accept();
      void CopyContext(SSLSocket * original);
      SSLSocket * AcceptConnection();

      const char * certfilename;
      const char * keyfilename;


   private:
      void Init(bool context);		// Defaults to create a client context, true if server context needed
      void InitContext();
      void InitServer();
      void InitServerContext();
      void LoadCertificates();

// Instance variables      
      void * SSLContext;				// SSL context
      void * SSLStruct;					// SSL BIO (Basic Input/Output)

};

#endif