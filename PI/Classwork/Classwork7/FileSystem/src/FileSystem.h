
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <cstring>
#include <iomanip>  // para std::hex y std::setw

const int TAMANIOBLOQUE = 64;
const int BLOQUEDIRECTORIO = 0;
const int CANTIDADBLOQUES = 16384;
const int TAMANIOBYTES = 1048576;
const int CANTBLOQUESBITMAP = 32;
const int BLOQUEINICIOCONTENIDO = 33;

struct Entrada {
    char nombre [10];
    int16_t indice;
    bool operator==(const std::string& name) const {
        return nombre == name;
    }
};

struct Indices {
    std::vector<int> indices;
    int bloqueIndiceSiguiente;
};


class FileSystem {
  public:
    FileSystem(bool crearNuevoDisco = false);
    ~FileSystem();
    void crearDisco(uint64_t size_in_bytes);
    std::vector<Entrada> leerDirectorio();
    Entrada buscarEntradaPorNombre(const std::string& nombreBuscado);
    std::string leerTxtAString(const std::string& rutaArchivo);
    bool guardarArchivo(const std::string& nombre, const std::string& contenido); //   Falta implementar
    size_t obtenerTamanoString(const std::string& figura);
    Indices leerBloqueIndice(int bloque);
    void guardarEntradaDirectorio(const std::string& nombreArchivo, int bloqueIndice);
    void guardarContenidoArchivo();
    void escribirBloquesDatos(const std::string& contenido, const std::vector<int>& bloquesDatos);
    void escribirBloqueIndice(int bloqueIndice, const std::vector<int>& bloquesDatos);
    Indices imprimirBloqueComoIndice(int blockNumber);
    std::string leerArchivoDesdeIndice(int bloqueIndice);

    // Manejo del bitmap
    void inicializarBitmap();
    void cargarBitmap();
    void guardarBitmap();
    void marcarBloquesOcupados(const std::vector<int>& bloques);
    void marcarBloquesLibres(const std::vector<int>& bloques);
    bool estaLibre(int bloque);
    void imprimirBitmap();

    std::string nombreDisco = "disco.bin";
    std::vector<Entrada> entradas;

    // Bitmap en memoria
    std::vector<uint8_t> bitmap; // cada bit representa un bloque



  private:


};