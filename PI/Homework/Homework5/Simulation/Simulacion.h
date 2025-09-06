#ifndef SIMULACION_H
#define SIMULACION_H

#include <iostream>
#include <string>
#include <thread>
#include <semaphore.h>

using namespace std;

// Buffers compartidos
extern string buffer_clienteTenedor;
extern string buffer_tenedorServidor;

// Semáforos
extern sem_t sem_clienteTenedor;     // Cliente envió solicitud al tenedor
extern sem_t sem_respuestaCliente;   // Tenedor envió respuesta al cliente
extern sem_t sem_tenedorServidor;    // Tenedor envió solicitud al servidor
extern sem_t sem_respuestaServidor;  // Servidor envió respuesta al tenedor

// Funciones
void cliente();
void tenedor();
void servidor();

#endif
