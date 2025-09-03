#include "Simulacion.h"

int main() {
    // Inicializar semáforos (0 significa que empieza bloqueado)
    sem_init(&sem_clienteTenedor, 0, 0);
    sem_init(&sem_respuestaCliente, 0, 0);
    sem_init(&sem_tenedorServidor, 0, 0);
    sem_init(&sem_respuestaServidor, 0, 0);

    // Crear hilos
    thread hilo_cliente(cliente);
    thread hilo_tenedor(tenedor);
    thread hilo_servidor(servidor);

    // Esperar que terminen
    hilo_cliente.join();
    hilo_tenedor.join();
    hilo_servidor.join();

    // Destruir semáforos
    sem_destroy(&sem_clienteTenedor);
    sem_destroy(&sem_respuestaCliente);
    sem_destroy(&sem_tenedorServidor);
    sem_destroy(&sem_respuestaServidor);

    return 0;
}
