#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <limits>   // para std::numeric_limits

#include "../common/tsq.hpp"
#include "../core/tenedor.hpp"
#include "../core/servidor.hpp"
#include "../common/Client.hpp"
#include "../core/FileSystem/FileSystem.h"
#include "../core/CacheServer/CacheServer.hpp"
// Lee archivo del host a un std::string (binario)
static bool read_file_to_string(const std::string& path, std::string& out) {
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (!ifs) return false;
  out.assign((std::istreambuf_iterator<char>(ifs)),
             std::istreambuf_iterator<char>());
  return true;
}

int main(int argc, char* argv[]) {
    // parámetros y filesystem
    if (argc < 2) {
    std::cout << "Uso: " << argv[0] << " <crear_disco: 1 = true, 0 = false>\n";
    return 1;  
    }
    const bool crearDisco = std::string(argv[1]) == "1";
    FileSystem fs(crearDisco);

    // === 6 colas ===
    auto pubReqQ   = std::make_shared<TSQ<std::string>>(); // público: req C->T
    auto pubRespQ  = std::make_shared<TSQ<std::string>>(); // público: resp T->C
    auto privReqQ  = std::make_shared<TSQ<DealerMsg>>();   // privado: req T->S 
    auto privRespQ = std::make_shared<TSQ<Reply>>();       // privado: resp S->T
    auto cacheReqQ = std::make_shared<TSQ<CacheReq>>();    // privado: req T->C
    auto cacheRespQ= std::make_shared<TSQ<CacheReq>>();    // privado: resp C->T

    // Instancias
    Tenedor  tenedor(pubReqQ, pubRespQ, privReqQ, privRespQ);
    Servidor servidor(privReqQ, privRespQ, cacheReqQ, cacheRespQ, &fs, "srv-1", "vlan-A");
    CacheServer cache(cacheReqQ, cacheRespQ, "cache-1");
    // Arranque
    servidor.startThread();
    tenedor.startThread();
    cache.startThread();

    // Cliente unico
    const int CID = 1;
    Client client(CID, pubReqQ, pubRespQ);
    client.startThread();

    // Menú interactivo
    for (;;) {
        std::cout << "\n=== CLIENTE ===\n"
            << "1) LIST\n"
            << "2) SUBMIT archivo\n"
            << "3) SUBMIT texto\n"
            << "4) REQUEST archivo\n"
            << "5) DELETE archivo\n"
            << "0) Salir\n"
            << "Seleccione una opcion: ";

        int op = -1;
        if (!(std::cin >> op)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada invalida\n";
            continue;
        }
        // limpiar el \n pendiente antes de getline()
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (op == 0) break;

        switch (op) {
            case 1: { // listar
                client.send("LIST");
                break;
            }
            case 2: { // subir desde host
                std::string hostPath, nombre;
                std::cout << "Ruta del archivo en host: ";
                std::getline(std::cin, hostPath);
                std::cout << "Nombre en servidor: ";
                std::getline(std::cin, nombre);

                std::string body;
                if (!read_file_to_string(hostPath, body)) {
                    std::cout << "No se pudo leer el archivo: " << hostPath << "\n";
                    break;
                }
                // Nota: protocolo simple "SUBMIT nombre|contenido"
                client.send("SUBMIT " + nombre + "|" + body);
                break;
            }
            case 3: { // subir texto directo
                std::string nombre, body;
                std::cout << "Nombre en servidor: ";
                std::getline(std::cin, nombre);
                std::cout << "Contenido:\n> ";
                std::getline(std::cin, body);
                client.send("SUBMIT " + nombre + "|" + body);
                break;
            }
            case 4: { // descargar
                std::string nombre;
                std::cout << "Nombre del archivo a descargar: ";
                std::getline(std::cin, nombre);

                int nbytes = 0;
                std::cout << "¿Cuántos bytes leer (nbytes)? ";
                if (!(std::cin >> nbytes) || nbytes <= 0) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "nbytes inválido\n";
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // limpiar \n

                // Protocolo: REQUEST <nombre>|<nbytes>
                client.send("REQUEST " + nombre + "|" + std::to_string(nbytes));
                break;
            }
      
            case 5: { // borrar
                std::string nombre;
                std::cout << "Nombre del archivo a borrar: ";
                std::getline(std::cin, nombre);
                client.send("DELETE " + nombre);
                break;
            }
            default:
            std::cout << "Opcion invalida\n";
            break;
        }
    }

    // Apagado ordenado
    client.stop();
    tenedor.stop();
    servidor.stop();

    client.waitToFinish();
    tenedor.waitToFinish();
    servidor.waitToFinish();

    std::cout << "Programa terminado\n";
    return 0;
}
