#include <iostream>
#include <string>
#include <vector>
#include "FileSystem.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " <crear_disco: 1 = true, 0 = false>\n";
        return 1;
    }

    bool crearDisco = std::string(argv[1]) == "1";

    // Crear FileSystem, pasar true para inicializar disco
    FileSystem* fs = new FileSystem(crearDisco);

    while (true) {
        std::cout << "\n====== MENÚ FILESYSTEM ======\n";
        std::cout << "1. Guardar archivo\n";
        std::cout << "2. Leer archivo\n";
        std::cout << "3. Imprimir bitmap\n";
        std::cout << "4. Leer directorio\n";
        std::cout << "0. Salir\n";
        std::cout << "Seleccione una opción: ";

        int opcion;
        std::cin >> opcion;
        std::cin.ignore(); // limpiar buffer

        try {
            if (opcion == 0) break;

            switch (opcion) {
                case 1: {
                    std::string archivoHost, nombreEnDisco;
                    std::cout << "Archivo en host: ";
                    std::getline(std::cin, archivoHost);
                    std::cout << "Nombre en disco: ";
                    std::getline(std::cin, nombreEnDisco);

                    if (fs->guardarArchivo(archivoHost, nombreEnDisco)) {
                        std::cout << "Archivo guardado correctamente.\n";
                    } else {
                        std::cout << "Error al guardar el archivo.\n";
                    }
                    break;
                }

                case 2: {
                    std::string nombreEnDisco;
                    std::cout << "Nombre del archivo en disco: ";
                    std::getline(std::cin, nombreEnDisco);

                    Entrada e = fs->buscarEntradaPorNombre(nombreEnDisco);
                    std::string contenido = fs->leerArchivoDesdeIndice(e.indice);
                    std::cout << "Contenido del archivo:\n" << contenido << "\n";
                    break;
                }

                case 3:
                    fs->cargarBitmap();
                    fs->imprimirBitmap();
                    break;

                case 4: {
                    std::vector<Entrada> directorio = fs->leerDirectorio();
                    std::cout << "Entradas en el directorio:\n";
                    for (const auto& entrada : directorio) {
                        std::cout << "- " << entrada.nombre << " -> bloque índice: " << entrada.indice << "\n";
                    }
                    break;
                }

                default:
                    std::cout << "Opción inválida.\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << "\n";
        }
    }

    delete fs;
    std::cout << "Programa terminado.\n";
    return 0;
}
