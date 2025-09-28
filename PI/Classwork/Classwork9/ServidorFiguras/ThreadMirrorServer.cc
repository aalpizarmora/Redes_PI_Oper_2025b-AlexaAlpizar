/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *   Socket client/server example with threads
  *
  * (Fedora version)
  *
 **/
 
#include <iostream>
#include <thread>

#include "Socket.h"
#include "FileSystem.h"

#define PORT 1234
#define BUFSIZE 512


/**
 *   Task each new thread will run
 *      Read string from socket
 *      Write it back to client
 *
 **/
void task(VSocket* client) {
    char a[BUFSIZE];


    // Crear instancia del sistema de archivos
    FileSystem fs(false);

    // Leer comando/nombre de archivo del cliente
    client->Read(a, BUFSIZE); // Read a string from client, data will be limited by BUFSIZE bytes
    std::cout << "Server received: " << a << std::endl;

    if (strcmp(a, "lista") == 0) {
        // Cliente pidió lista de archivos
        std::vector<Entrada> entradas = fs.leerDirectorio();
        std::string lista;
        for (const auto& entrada : entradas) {
            lista += std::string(entrada.nombre) + "\n";
        }
        client->Write(lista.c_str()); // Write it back to client, this is the mirror function
    } else {
        // Cliente pidió un archivo específico
        Entrada entrada = fs.buscarEntradaPorNombre(a);
        if (entrada.indice != -1) {
            std::string contenido = fs.leerArchivoDesdeIndice(entrada.indice);
            client->Write(contenido.c_str());
        } else {
            std::string mensaje = "Archivo no encontrado.\n";
            client->Write(mensaje.c_str());
        }
    }

    client->Close();  // Close socket in parent cliente
}



/**
 *   Create server code
 *      Infinite for
 *         Wait for client conection
 *         Spawn a new thread to handle client request
 *
 **/
int main( int argc, char ** argv ) {
   std::thread * worker;
   VSocket * s1, * client;

   s1 = new Socket( 's' );

   s1->Bind( PORT );		// Port to access this mirror server
   s1->MarkPassive( 5 );	// Set socket passive and backlog queue to 5 connections

   for( ; ; ) {
      client = s1->AcceptConnection();	 	// Wait for a client connection
      worker = new std::thread( task, client );
   }

}
