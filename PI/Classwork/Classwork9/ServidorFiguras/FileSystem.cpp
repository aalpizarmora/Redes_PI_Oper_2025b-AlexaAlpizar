#include "FileSystem.h"

FileSystem::FileSystem(bool crearNuevoDisco) {

    if (crearNuevoDisco) {
        crearDisco(TAMANIOBYTES); // Crear disco de 1MB
        // Inicializar bitmap en ceros
        inicializarBitmap();
    }
    inicializarBitmap();
    // Cargar el bitmap en memoria
    cargarBitmap();
}

FileSystem::~FileSystem() {
    // Guardar el bitmap en disco al cerrar
    guardarBitmap();
}

void FileSystem::crearDisco(uint64_t size_in_bytes) {
    std::ofstream disk(this->nombreDisco, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("Error: No se pudo crear el archivo del disco.\n");
    }

    std::vector<char> buffer(TAMANIOBLOQUE, 0);
    std::uint64_t written = 0;

    while (written < size_in_bytes) {
        std::uint64_t to_write = std::min<std::uint64_t>(TAMANIOBLOQUE, size_in_bytes - written);
        disk.write(buffer.data(), to_write);
        written += to_write;
    }

    disk.close();
    std::cout << "Disco creado: " << this->nombreDisco << " (" << size_in_bytes << " bytes)\n";
}


std::vector<Entrada> FileSystem::leerDirectorio(){
    // Función para leer el directorio desde disco
    std::ifstream disk(this->nombreDisco, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("Error: No se pudo abrir el disco.\n");
    }

    // Ir al bloque reservado para el directorio
    disk.seekg(BLOQUEDIRECTORIO * TAMANIOBLOQUE, std::ios::beg);

    // Calcular cuántas entradas caben en un bloque
    size_t maxEntradas = TAMANIOBLOQUE / sizeof(Entrada);

    this->entradas.clear();
    for (size_t i = 0; i < maxEntradas; i++) {
        Entrada e;
        disk.read(reinterpret_cast<char*>(&e), sizeof(Entrada));

        // Si el nombre está vacío, ya no hay más entradas
        if (e.nombre[0] == '\0') {
            break;
        }

        this->entradas.push_back(e);
    }

    disk.close();
    std::cout << "Directorio leído. Entradas: " << this->entradas.size() << "\n";
    return this->entradas;
}

// Función para buscar una entrada por nombre en el directorio
// Retorna true si la encuentra y coloca el índice en "indiceEncontrado"
Entrada FileSystem::buscarEntradaPorNombre(const std::string& nombreBuscado) {
    std::ifstream disk(this->nombreDisco, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("Error: No se pudo abrir el disco.\n");

    }

    disk.seekg(BLOQUEDIRECTORIO * TAMANIOBLOQUE, std::ios::beg);
    size_t maxEntradas = TAMANIOBLOQUE / sizeof(Entrada);
    for (size_t i = 0; i < maxEntradas; i++) {
        Entrada e;
        disk.read(reinterpret_cast<char*>(&e), sizeof(Entrada));

        if (e.nombre[0] == '\0') continue; // entrada vacía

        if (std::string(e.nombre) == nombreBuscado) {
            disk.close();
            std::cout << "Archivo encontrado" << "\n" ;
            return e; // encontrada
        }
    }

    disk.close();
    std::cout << "Archivo no existe" << "\n";
    return Entrada{"", -1}; // no encontrada
}

// Lee un archivo .txt y devuelve su contenido como un string
std::string FileSystem::leerTxtAString(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + rutaArchivo);
    }

    std::stringstream buffer;
    buffer << archivo.rdbuf();  // Lee todo el contenido
    archivo.close();
    return buffer.str();
}


bool FileSystem::guardarArchivo(const std::string& rutaArchivo, const std::string& nombreArchivo) {
    // Leer contenido del archivo
    std::string contenido = leerTxtAString(rutaArchivo);
    std::cout << "Contenido del archivo a guardar (" << contenido.size() << " bytes)\n";

    // Calcular cuántos bloques necesita
    size_t bloquesNecesarios = (contenido.size() + TAMANIOBLOQUE - 1) / TAMANIOBLOQUE;
    std::cout << "Bloques necesarios: " << bloquesNecesarios << "\n";

    // +1 bloque para el bloque indice donde se almacenan los indices de los bloques de datos
    size_t totalBloques = bloquesNecesarios + 1;
    std::cout << "Total bloques a asignar (incluyendo índice): " << totalBloques << "\n";

    // Buscar bloques libres
    std::vector<int> bloquesAsignados;
    int totalBits = bitmap.size() * 8;
    for (int i = 0; i < totalBits && bloquesAsignados.size() < totalBloques; i++) {
        if (estaLibre(i)) {
            std::cout << "Bloque libre encontrado: " << i << "\n";
            bloquesAsignados.push_back(i);
        }
    }

    if (bloquesAsignados.size() < totalBloques) {
        std::cerr << "Error: No hay suficiente espacio en disco.\n";
        return false;
    }

    // El primer bloque de la lista será el índice
    int bloqueIndice = bloquesAsignados[0];
    std::vector<int> bloquesDatos(bloquesAsignados.begin() + 1, bloquesAsignados.end());

    //Marcar bloques como ocupados
    marcarBloquesOcupados(bloquesAsignados);
    guardarBitmap();

    this->imprimirBitmap();

    escribirBloqueIndice(bloqueIndice, bloquesDatos);

    this->imprimirBitmap();

    escribirBloquesDatos(contenido, bloquesDatos);

    // Guardar entrada en el directorio
    guardarEntradaDirectorio(nombreArchivo, bloqueIndice);
    std::cout << "Archivo '" << nombreArchivo << "' guardado correctamente en el disco.\n";
    return true;
}

void FileSystem::escribirBloquesDatos(const std::string& contenido, const std::vector<int>& bloquesDatos) {
    std::fstream disk(this->nombreDisco, std::ios::binary | std::ios::in | std::ios::out);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para escribir archivo.");
    }

    std::cout << "Escribiendo contenido en bloques de datos...\n";

    size_t offset = 0;

    for (int bloque : bloquesDatos) {
        std::cout << "Escribiendo en bloque de datos: " << bloque << "\n";
        disk.seekp(bloque * TAMANIOBLOQUE, std::ios::beg);

        size_t bytesAEscribir = std::min((size_t)TAMANIOBLOQUE, contenido.size() - offset);
        disk.write(contenido.data() + offset, bytesAEscribir);

        // Rellenar con ceros si sobra espacio en el bloque
        if (bytesAEscribir < TAMANIOBLOQUE) {
            std::vector<char> relleno(TAMANIOBLOQUE - bytesAEscribir, 0);
            disk.write(relleno.data(), relleno.size());
        }

        offset += bytesAEscribir;
    }

    disk.close();
}

void FileSystem::escribirBloqueIndice(int bloqueIndice, const std::vector<int>& bloquesDatos) {
    std::fstream disk(this->nombreDisco, std::ios::binary | std::ios::in | std::ios::out);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para escribir archivo.");
    }
    std::cout << "Escribiendo bloque índice en: " << bloqueIndice << "\n";

    // Cantidad máxima de enteros que caben en el bloque
    size_t maxInts = TAMANIOBLOQUE / sizeof(int);

    // El último entero es reservado para "bloqueIndiceSiguiente"
    size_t maxBloquesDatos = maxInts - 1;

    // Crear buffer con todo el bloque en 0
    std::vector<int> buffer(maxInts, 0);

    // Copiar los bloques de datos
    for (size_t i = 0; i < bloquesDatos.size() && i < maxBloquesDatos; i++) {
        buffer[i] = bloquesDatos[i];
    }

    // Último entero = siguiente bloque índice (por ahora -1)
    buffer[maxInts - 1] = -1;

    // Escribir el bloque completo
    disk.seekp(bloqueIndice * TAMANIOBLOQUE, std::ios::beg);
    disk.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int));
    disk.close();

    Indices indice = imprimirBloqueComoIndice(bloqueIndice);
    std::cout << "Bloque índice escrito correctamente.\n";
    std::cout << "Índices en el bloque índice:\n";
    for (size_t i = 0; i < maxBloquesDatos; i++) {
        if (buffer[i] != 0) {
            std::cout << "Indice[" << i << "] = " << buffer[i] << "\n";
        }
    }
    std::cout << "Siguiente bloque índice: " << buffer[maxInts - 1] << "\n";
}

// Función para imprimir un bloque específico
Indices FileSystem::imprimirBloqueComoIndice(int numeroBloque) {
    Indices indiceStruct;
    std::ifstream disco(this->nombreDisco, std::ios::binary);
    if (!disco) {
        throw std::runtime_error("Error: no se pudo abrir el disco.");
    }

    // Mover al inicio del bloque índice
    disco.seekg(numeroBloque * TAMANIOBLOQUE, std::ios::beg);

    // Calcular cuántos enteros caben en un bloque, dejando espacio para el puntero siguiente
    size_t cantidadInts = TAMANIOBLOQUE / sizeof(int) - 1;

    indiceStruct.indices.resize(cantidadInts);

    // Leer todos los índices
    disco.read(reinterpret_cast<char*>(indiceStruct.indices.data()), cantidadInts * sizeof(int));

    // Leer el siguiente bloque índice
    disco.read(reinterpret_cast<char*>(&indiceStruct.bloqueIndiceSiguiente), sizeof(int));

    disco.close();
    return indiceStruct;
}

void FileSystem::guardarEntradaDirectorio(const std::string& nombreArchivo, int bloqueIndice) {
    std::fstream disk(this->nombreDisco, std::ios::binary | std::ios::in | std::ios::out);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para escribir archivo.");
    }
    
    Entrada nuevaEntrada;
    std::memset(nuevaEntrada.nombre, 0, sizeof(nuevaEntrada.nombre));
    std::strncpy(nuevaEntrada.nombre, nombreArchivo.c_str(), sizeof(nuevaEntrada.nombre) - 1);
    nuevaEntrada.indice = bloqueIndice;

    // Ir al bloque del directorio (bloque 0)
    disk.seekp(BLOQUEDIRECTORIO * TAMANIOBLOQUE, std::ios::beg);

    size_t maxEntradas = TAMANIOBLOQUE / sizeof(Entrada);
    for (size_t i = 0; i < maxEntradas; i++) {
        Entrada e;
        disk.read(reinterpret_cast<char*>(&e), sizeof(Entrada));

        if (e.nombre[0] == '\0') {
            // Reutilizar espacio vacío
            disk.seekp(BLOQUEDIRECTORIO * TAMANIOBLOQUE + i * sizeof(Entrada), std::ios::beg);
            disk.write(reinterpret_cast<char*>(&nuevaEntrada), sizeof(Entrada));
            break;
        }
    }
    std::cout << "Archivo '" << nombreArchivo << "' guardado correctamente en el directorio.\n";
}

void FileSystem::guardarContenidoArchivo() {
    
}



Indices FileSystem::leerBloqueIndice(int bloque) {
    if (bloque < 5) {
        throw std::invalid_argument("Los bloques de índice empiezan a partir del bloque 5.");
    }

    std::ifstream disk(this->nombreDisco, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para leer el bloque índice.");
    }

    // Mover puntero al inicio del bloque
    disk.seekg(bloque * TAMANIOBLOQUE, std::ios::beg);

    // Leer 64 bytes que componen el bloque índice
    std::vector<char> buffer(64, 0);
    disk.read(buffer.data(), buffer.size());
    disk.close();

    Indices indice;
    indice.indices.clear();

    // Cada bloque índice puede almacenar varios enteros (4 bytes c/u)
    // menos 4 bytes finales que se reservan para bloqueIndiceSiguiente
    size_t capacidadEnteros = (64 - sizeof(int)) / sizeof(int);

    const int* data = reinterpret_cast<const int*>(buffer.data());

    for (size_t i = 0; i < capacidadEnteros; i++) {
        if (data[i] == 0) break; // cero = no hay más bloques
        indice.indices.push_back(data[i]);
    }

    // último entero del bloque = puntero al siguiente bloque índice
    indice.bloqueIndiceSiguiente = data[capacidadEnteros];

    return indice;
}

std::string FileSystem::leerArchivoDesdeIndice(int bloqueIndice) {
    // Leer el bloque índice
    Indices indice = leerBloqueIndice(bloqueIndice);

    std::fstream disk(this->nombreDisco, std::ios::binary | std::ios::in);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para leer el archivo.");
    }

    std::string contenido;

    // Recorrer los bloques de datos listados en el bloque índice
    for (int bloqueDatos : indice.indices) {
        if (bloqueDatos == 0) continue; // bloques vacíos en el índice

        disk.seekg(bloqueDatos * TAMANIOBLOQUE, std::ios::beg);

        std::vector<char> buffer(TAMANIOBLOQUE);
        disk.read(buffer.data(), TAMANIOBLOQUE);

        // Agregar al contenido final, eliminando posibles ceros al final
        for (char c : buffer) {
            if (c == '\0') break;
            contenido.push_back(c);
        }
    }

    disk.close();
    return contenido;
}

// Devuelve el tamaño (número de caracteres) de un string
size_t FileSystem::obtenerTamanoString(const std::string& figura) {
    return figura.size(); // o texto.length()
}


// Inicializar el bitmap en ceros (todo libre) y guardarlo en disco
void FileSystem::inicializarBitmap() {
    // Supongamos que el bitmap ocupa 32 bloques de 256 bytes cada uno
    size_t bitmapSize = CANTBLOQUESBITMAP * TAMANIOBLOQUE; // 32 * 64 = 2048 bytes
    bitmap.assign(bitmapSize, 0); // iniciar todo en cero

    // Marcar los primeros 33 bloques como ocupados
    for (int i = 0; i < CANTBLOQUESBITMAP+1; i++) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        bitmap[byteIndex] |= (1 << bitIndex); // poner bit en 1
    }

    // Guardar en disco (empezando en el bloque 1, porque bloque 0 = directorio)
    std::ofstream disk(this->nombreDisco, std::ios::binary | std::ios::in | std::ios::out);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para inicializar bitmap.");
    }

    disk.seekp(1 * TAMANIOBLOQUE, std::ios::beg); // bloque 1
    disk.write(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    disk.close();
    std::cout << "Bitmap inicializado. Primeros 33 bloques ocupados.\n";
}

// Cargar el bitmap desde los bloques 2 al 6
void FileSystem::cargarBitmap() {
    std::ifstream disk(this->nombreDisco, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para leer bitmap.");
    }

    // Bitmap ocupa 32 bloques
    size_t bitmapSize = CANTBLOQUESBITMAP * TAMANIOBLOQUE;
    bitmap.resize(bitmapSize);

    // El bitmap empieza en el bloque 1, multiplicamos por el tamaño de bloque
    disk.seekg(1 * TAMANIOBLOQUE, std::ios::beg);
    disk.read(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    disk.close();
    std::cout << "Bitmap cargado (" << bitmap.size() << " bytes)\n";
}


// Guardar el bitmap en disco (para reflejar cambios)
void FileSystem::guardarBitmap() {
    std::ofstream disk(this->nombreDisco, std::ios::binary | std::ios::in | std::ios::out);
    if (!disk) {
        throw std::runtime_error("No se pudo abrir el disco para escribir bitmap.");
    }

    size_t bitmapSize = bitmap.size();
    disk.seekp(1 * TAMANIOBLOQUE, std::ios::beg);
    disk.write(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    disk.close();
    std::cout << "Bitmap guardado.\n";
}


// Marcar bloques como ocupados en el bitmap
void FileSystem::marcarBloquesOcupados(const std::vector<int>& bloques) {
    for (int bloque : bloques) {
        int byteIndex = bloque / 8;
        int bitIndex = bloque % 8;

        if (byteIndex >= bitmap.size()) {
            throw std::out_of_range("Bloque fuera del rango representado por el bitmap.");
        }

        bitmap[byteIndex] |= (1 << bitIndex); // marcar bit en 1
        std::cout << "Marcando bloque " << bloque << " como ocupado.\n";
    }
    std::cout << "Bloques marcados como ocupados.\n";
}

// Marcar bloques como libres en el bitmap
void FileSystem::marcarBloquesLibres(const std::vector<int>& bloques) {
    for (int bloque : bloques) {
        size_t byteIndex = bloque / 8;
        int bitIndex = bloque % 8;

        if (byteIndex >= bitmap.size()) {
            throw std::out_of_range("Bloque fuera del rango representado por el bitmap.");
        }

        bitmap[byteIndex] &= ~(1 << bitIndex); // poner bit en 0
    }
    std::cout << "Bloques marcados como libres.\n";
}

// Verificar si un bloque está libre
bool FileSystem::estaLibre(int bloque) {
    int byteIndex = bloque / 8;
    int bitIndex = bloque % 8;

    if (byteIndex >= bitmap.size()) {
        throw std::out_of_range("Bloque fuera del rango representado por el bitmap.");
    }

    return !(bitmap[byteIndex] & (1 << bitIndex)); // true si está libre
}

// Imprimir el estado del bitmap
void FileSystem::imprimirBitmap() {
    std::cout << "Estado del bitmap (0 = libre, 1 = ocupado):\n";

    int totalBits = bitmap.size() * 8;
    for (int i = 0; i < totalBits; i++) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;

        bool ocupado = bitmap[byteIndex] & (1 << bitIndex);

        std::cout << (ocupado ? "1" : "0");

        if ((i + 1) % 8 == 0) std::cout << " ";     // separar por bytes
        if ((i + 1) % 64 == 0) std::cout << "\n";  // salto cada 64 bits (8 bytes)
    }
    std::cout << "\n";
}