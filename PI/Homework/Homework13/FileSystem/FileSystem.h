#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

/**
 * @brief Tamaño de cada bloque en bytes.
 */
const int TBLOQUE = 256;

/**
 * @brief Bloque reservado para el directorio raíz.
 */
const int BLOQUEDIRECTORIO = 0;

/**
 * @brief Tamaño total del disco en bytes.
 */
const int TUNIDAD = 1024; // para hacer pruebas
//const int TUNIDAD = 1048576;  // 1 MB

/**
 * @brief Cantidad total de bloques en el disco.
 */
const int CANTIDADBLOQUES = TUNIDAD / TBLOQUE;  // 1 MB entre 256 bytes

/**
 * @brief Cantidad de bloques reservados para almacenar el bitmap.
 */
const int CANTBLOQUESBITMAP = 32;

/**
 * @brief Bloque donde comienza el área de contenido de archivos.
 */
const int BLOQUEINICIOCONTENIDO = 33;

/**
 * @brief Tamanio del arreglo directorio en bytes
 */
const int TDIRECTORIO = 3;

/**
 * @struct Entrada
 * @brief Representa una entrada de directorio en el sistema de archivos.
 */
struct Entrada {
    char nombre [10];   ///< Nombre del archivo (máximo 10 caracteres).
    int16_t indice;     ///< Número de bloque índice asociado al archivo.
};

/**
 * @struct Indices
 * @brief Representa un bloque índice que almacena referencias a bloques de datos.
 */
struct Indices {
    std::vector<int> indices; ///< Lista de bloques de datos asociados al archivo.
    int bloqueIndiceSiguiente; ///< Número del siguiente bloque índice (si existe).
};

class INodo {
  private:
    int bloque0;
    int bloque1;
    int bloque2;
    int bloque3;
    // TODO(any): implementar bloques indirectos
    INodo* bloqueIndirecto0;
    INodo* bloqueIndirecto1;

  bool operator==(const INodo& other) const {
    return this->nombre == other.nombre;
  }
  
  public:
    std::string nombre;
    std::string fecha;

    INodo();
    ~INodo();

    bool asignarBloque(int bloque);
    int obtenerBloque0() { return this->bloque0; }
    int obtenerBloque1() { return this->bloque1; }
    int obtenerBloque2() { return this->bloque2; }
    int obtenerBloque3() { return this->bloque3; }
    int obtenerUltimoBloque();
};

struct Bloque {
  char datos[TBLOQUE]; // 256 bytes
};

/**
 * @class FileSystem
 * @brief Clase que implementa un sistema de archivos simple con directorio y bitmap.
 */

// TODO(any): journaling?
class FileSystem {
  public:
    /**
     * @brief Constructor de FileSystem.
     * @param crearNuevoDisco true para crear un nuevo disco, false para cargar uno existente.
     */
    FileSystem(bool crearNuevoDisco = false);

    /**
     * @brief Destructor de FileSystem.
     */
    ~FileSystem();

    /**
     * @brief Crea un archivo de disco binario con el tamaño especificado.
     * @param size_in_bytes Tamaño total del disco en bytes.
     */
    void crearDisco(uint64_t size_in_bytes);

    /**
     * @brief Lee el contenido del directorio.
     * @return Vector de entradas de directorio.
     */
    std::vector<Entrada> leerDirectorio();

    /**
     * @brief Verifica si existe un archivo con ese nombre.
     * @return true si hay un archivo con ese nombre; false si no.
     */
    bool nombreExiste(std::string nombre);

    // ------------------- Manejo de archivos -------------------

    /**
     * @brief Guarda un archivo en el sistema de archivos.
     * @param nombre Nombre del archivo.
     * @param contenido Contenido del archivo como string.
     * @return true si se guardó con éxito, false en caso contrario.
     */
    bool guardarArchivo(const std::string& nombre, const std::string& contenido);

    /**
     * @brief Busca una entrada de directorio por nombre.
     * @param nombreBuscado Nombre del archivo a buscar.
     * @return Entrada encontrada (si no existe, debe manejarse en implementación).
     */
    int buscarArchivoPorNombre(const std::string& nombre);

    /**
     * @brief Agrega (append) un caracter al final del archivo.
     * @param nombre Nombre del archivo.
     * @param byte El byte que contiene el caracter por agregar.
     * @return true si logra agregar el caracter; false si no.
     */
    bool agregar(std::string nombre, char byte);

    /**
     * @brief Lee 'nbytes' de un archivo.
     * @param nombre Nombre del archivo.
     * @param nbytes La cantidad de bytes que se van a leer.
     * @return Arreglo de caracteres que contiene el contenido leído.
     */
    char* leer(std::string nombre, int nbytes);

    /**
     * @brief Escribe datos en un archivo, sobrescribiendo el contenido anterior.
     * @param nombre Nombre del archivo.
     * @param datos Los datos por escribir en el archivo.
     * @return true si logra escribir en el archivo; false si no.
     * @todo Diferenciar este método de 'modificar'.
     */
    bool escribir(std::string nombre, const char* datos);

    /**
     * @brief Renombra un archivo.
     * @param nombre Nombre original del archivo.
     * @param nombreNuevo Nombre nuevo del archivo al que se va a cambiar.
     * @return true si logra renombrar el archivo; false si no.
     */
    bool renombrar(std::string nombre, std::string nombreNuevo);

    /**
     * @brief Elimina un archivo del File System.
     * @param nombre Nombre del archivo.
     * @return true si logra eliminar el archivo; false si no.
     * @note Si se intenta eliminar un archivo que no existe, devolverá false.
     */
    bool eliminar(std::string nombre);

    /**
     * @brief Modifica los datos de un archivo, sobrescribiéndolos.
     * @param nombre Nombre del archivo.
     * @param datos Los datos que se escribirán en el archivo.
     * @return true si logra modificar el archivo; false si no.
     * @todo Diferenciar este método de 'escribir'.
     */
    bool modificar(std::string nombre, const char* datos);
  
    /**
     * @brief Reemplaza los datos de un archivo en la estructura interna del FS.
     * @detail A diferencia de 'modificar', este método no solo sobreescribe el archivo,
     *  sino que también escribe los datos nuevos en bloques distintos. En esencia,
     *  este método crea un archivo nuevo con el mismo nombre que el del anterior.
     * @detail Puede ser útil en casos de fragmentación externa.
     * @param nombre Nombre del archivo.
     * @param datosNuevos Los datos que se escribirán en el archivo.
     * @return true si logra reemplazar el archivo; false si no.
     * @todo Hacer que sea posible no especificar 'datosNuevos', de manera que el reemplazo
     *  es sólo de bloques, y no también de contenido (que haya la opción para ambas cosas)
     */
    bool reemplazar(std::string nombre, const char* datosNuevos);

    // ------------------- Manejo de inodos -------------------

    /**
     * @brief Crea un INodo en el FileSystem
     * @param nombre Nombre que se asociará al INodo (nombre del archivo)
     */
    void crearInodo(std::string nombre);

    // ------------------- Manejo del bitmap -------------------

    /**
     * @brief Inicializa el bitmap en memoria, marcando los bloques reservados.
     */
    void inicializarBitmap();

    /**
     * @brief Carga el bitmap desde el disco hacia la memoria.
     */
    void cargarBitmap();

    /**
     * @brief Guarda el bitmap en disco desde la memoria.
     */
    void guardarBitmap();

    /**
     * @brief Obtiene un bloque por índice.
     * @param bloque El índice del bloque.
     * @return Un puntero al bloque obtenido.
     */
    Bloque* obtenerBloque(int bloque);

    /**
     * @brief Guarda un bloque nuevo en una posición disponible del bitmap.
     * @param bloque El bloque por guardar.
     * @warning Sobreescribe forzosamente.
     */
    void guardarBloque(Bloque bloque[]);

    /**
     * @brief Busca un bloque libre.
     * @return El índice del bloque encontrado, o -1 si no hay bloque libre.
     */
    int buscarBloqueLibre();

    /**
     * @brief Marca una lista de bloques como ocupados en el bitmap.
     * @param bloques Vector con los números de bloque a marcar.
     */
    void marcarBloquesOcupados(const std::vector<int>& bloques);

    /**
     * @brief Marca una lista de bloques como libres en el bitmap.
     * @param bloques Vector con los números de bloque a liberar.
     */
    void marcarBloquesLibres(const std::vector<int>& bloques);

    /**
     * @brief Verifica si un bloque está libre.
     * @param bloque Número del bloque a verificar.
     * @return true si el bloque está libre, false si está ocupado.
     */
    bool estaLibre(int bloque);

    // ------------------- Impresiones -------------------

    /**
     * @brief Lee los INodos en la unidad.
     * @return Una cadena con la lista de INodos.
     */
    std::string leerUnidad();

    /**
     * @brief Imprime los contenidos de un directorio.
     */
    void imprimirDirectorio();

    /**
     * @brief Imprime los contenidos de la Unidad (la lista de INodos).
     */
    void imprimirUnidad();

    /**
     * @brief Imprime en consola el estado del bitmap.
     */
    void imprimirBitmap();

    /**
     * @brief Imprime toda la información del File System.
     * @details Llama a los demás métodos de impresión.
     */
    void imprimir();

  //abrir (lectura/escritura)
    //buscar
    //marcar como abierto
    //guardar el permiso

    //UGA
    //USER GROUP ALL
    //111 001 000
    //chmod 755 x.dat
    //111 101 101
  //borrar
  //renombrar
  //leer(nombre, posicion, tamano)
    //abrir
    //ubicarse en la posicion
    //leer esa cantidad de datos a partir de esa posicion

  //escribir?

  private:
    // +--------------------+
    // | ATRIBUTOS PRIVADOS |
    // +--------------------+

    /**
     * @brief Nombre del archivo que representa el disco en el sistema de archivos.
     */
    std::string nombreDisco = "disco.bin";

    /**
     * @brief Entradas del directorio en memoria.
     */
    std::vector<Entrada> entradas;

    /**
     * @brief El INodo directorio.
     */
    INodo* directorio;
    /**
     * @brief Bitmap en memoria. Cada bit representa un bloque (0/false = libre, 1/true = ocupado).
     */
    bool* bitmap;
    /**
     * @brief La unidad.
     */
    std::fstream unidad;
    /**
     * @brief El número de archivos en el File System.
     */
    int nFiles = 0;

    // +--------------------+
    // | MÉTODOS AUXILIARES |
    // +--------------------+
};
