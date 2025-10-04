#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <cstring>
#include <iomanip>  // para std::hex y std::setw

/// Tamaño de un bloque en bytes
const int TAMANIOBLOQUE = 64;
/// Número del bloque reservado para el directorio
const int BLOQUEDIRECTORIO = 0;
/// Cantidad total de bloques en el disco
const int CANTIDADBLOQUES = 16384;
/// Tamaño total del disco en bytes
const int TAMANIOBYTES = 1048576;
/// Cantidad de bloques reservados para el bitmap, 16384/8/64 = 32
const int CANTBLOQUESBITMAP = 32;
/// Primer bloque disponible para almacenar contenido
const int BLOQUEINICIOCONTENIDO = 33;

/**
 * @struct Entrada
 * @brief Representa una entrada en el directorio del sistema de archivos.
 */
struct Entrada {
    char nombre[10];      ///< Nombre del archivo (máx. 10 caracteres)
    int16_t indice;       ///< Índice del bloque que apunta al archivo

    /**
     * @brief Compara el nombre de la entrada con un nombre dado.
     * @param name Nombre a comparar.
     * @return true si son iguales, false en caso contrario.
     */
    bool operator==(const std::string& name) const {
        return nombre == name;
    }
};

/**
 * @struct Indices
 * @brief Representa un bloque de índices que apunta a bloques de datos.
 */
struct Indices {
    std::vector<int> indices;   ///< Vector con índices de bloques de datos
    int bloqueIndiceSiguiente;  ///< Bloque del siguiente índice (si aplica)
};

/**
 * @class FileSystem
 * @brief Clase que implementa un sistema de archivos simple sobre un disco virtual.
 */
class FileSystem {
  public:
    /**
     * @brief Constructor de FileSystem.
     * @param crearNuevoDisco Si es true, se crea un nuevo disco desde cero.
     */
    FileSystem(bool crearNuevoDisco = false);

    /**
     * @brief Destructor de FileSystem.
     */
    ~FileSystem();

    /**
     * @brief Crea un nuevo disco virtual en un archivo binario.
     * @param size_in_bytes Tamaño del disco en bytes.
     */
    void crearDisco(uint64_t size_in_bytes);

    /**
     * @brief Lee el directorio del disco.
     * @return Vector con todas las entradas del directorio.
     */
    std::vector<Entrada> leerDirectorio();

    /**
     * @brief Busca una entrada en el directorio por nombre.
     * @param nombreBuscado Nombre del archivo a buscar.
     * @return Entrada encontrada, si existe.
     */
    Entrada buscarEntradaPorNombre(const std::string& nombreBuscado);

    /**
     * @brief Lee un archivo de texto y lo convierte en un string.
     * @param rutaArchivo Ruta del archivo a leer.
     * @return Contenido del archivo como string.
     */
    std::string leerTxtAString(const std::string& rutaArchivo);

    /**
     * @brief Guarda un archivo en el sistema de archivos.
     * @param nombre Nombre del archivo.
     * @param contenido Contenido del archivo.
     * @return true si se guardó correctamente, false en caso contrario.
     */
    bool guardarArchivo(const std::string& nombre, const std::string& contenido);

    /**
     * @brief Obtiene el tamaño en bytes de un string.
     * @param figura String cuyo tamaño se desea calcular.
     * @return Tamaño en bytes.
     */
    size_t obtenerTamanoString(const std::string& figura);

    /**
     * @brief Lee un bloque de índices desde el disco.
     * @param bloque Número de bloque de índice.
     * @return Estructura Indices con los datos leídos.
     */
    Indices leerBloqueIndice(int bloque);

    /**
     * @brief Guarda una nueva entrada en el directorio.
     * @param nombreArchivo Nombre del archivo.
     * @param bloqueIndice Bloque índice asignado al archivo.
     */
    void guardarEntradaDirectorio(const std::string& nombreArchivo, int bloqueIndice);

    /**
     * @brief Guarda el contenido del archivo en bloques de datos.
     */
    void guardarContenidoArchivo();

    /**
     * @brief Escribe contenido en bloques de datos.
     * @param contenido Contenido a escribir.
     * @param bloquesDatos Lista de bloques donde escribir.
     */
    void escribirBloquesDatos(const std::string& contenido, const std::vector<int>& bloquesDatos);

    /**
     * @brief Escribe un bloque de índices en el disco.
     * @param bloqueIndice Número de bloque de índice.
     * @param bloquesDatos Lista de bloques de datos asociados.
     */
    void escribirBloqueIndice(int bloqueIndice, const std::vector<int>& bloquesDatos);

    /**
     * @brief Imprime el contenido de un bloque interpretado como índice.
     * @param blockNumber Número del bloque.
     * @return Estructura Indices con la información.
     */
    Indices imprimirBloqueComoIndice(int blockNumber);

    /**
     * @brief Lee un archivo a partir de su bloque índice.
     * @param bloqueIndice Número de bloque índice.
     * @return Contenido del archivo como string.
     */
    std::string leerArchivoDesdeIndice(int bloqueIndice);

    /** @name Manejo del Bitmap
     *  Funciones para gestionar el mapa de bits de bloques ocupados/libres.
     */
    ///@{
    /**
     * @brief Inicializa el bitmap en memoria.
     */
    void inicializarBitmap();

    /**
     * @brief Carga el bitmap desde el disco.
     */
    void cargarBitmap();

    /**
     * @brief Guarda el bitmap en el disco.
     */
    void guardarBitmap();

    /**
     * @brief Marca una lista de bloques como ocupados.
     * @param bloques Lista de bloques a marcar.
     */
    void marcarBloquesOcupados(const std::vector<int>& bloques);

    /**
     * @brief Marca una lista de bloques como libres.
     * @param bloques Lista de bloques a liberar.
     */
    void marcarBloquesLibres(const std::vector<int>& bloques);

    /**
     * @brief Verifica si un bloque está libre.
     * @param bloque Número de bloque.
     * @return true si el bloque está libre, false si está ocupado.
     */
    bool estaLibre(int bloque);

    /**
     * @brief Imprime el contenido del bitmap en consola.
     */
    void imprimirBitmap();
    ///@}

    std::string nombreDisco = "disco.bin"; ///< Nombre del archivo que representa el disco.
    std::vector<Entrada> entradas;         ///< Entradas de directorio cargadas en memoria.

    /// Bitmap en memoria, donde cada bit representa un bloque.
    std::vector<uint8_t> bitmap;

  private:
    // Atributos privados reservados para futuras implementaciones.
};
