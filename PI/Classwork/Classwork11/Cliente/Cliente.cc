#include "Cliente.h"
#include <iostream>
#include <cstring> // Para memset
#include <stdexcept> // Para excepciones

// Constructor por defecto
Cliente::Cliente() : puerto(80), IP("163.178.104.62"), socketCliente(nullptr) {
    std::cout << "Cliente creado con IP por defecto (" << IP 
              << ") y puerto por defecto (" << puerto << ")" << std::endl;
}

// Constructor con parámetros
Cliente::Cliente(const char* IP, int puerto) : puerto(puerto), IP(IP),socketCliente(nullptr) {
    std::cout << "Cliente creado con IP: " << IP 
              << " y puerto: " << puerto << std::endl;
}

// Destructor
Cliente::~Cliente() {
    if (socketCliente) {
        delete socketCliente;
    }
    std::cout << "Cliente destruido" << std::endl;
}

// Método principal para iniciar la conexión
void Cliente::start() {
    std::cout << "Iniciando cliente..." << std::endl;

    if (connectTCP() != 0) {
        std::cerr << "Error al conectar por TCP" << std::endl;
        return;
    }
    /*if (connectSSL() != 0) {
        std::cerr << "Error al conectar por SSL" << std::endl;
        return;
    }*/
   
}

// Conectar vía TCP
int Cliente::connectTCP() {
    std::cout << "Conectando a " << IP << ":" << puerto << " vía TCP..." << std::endl;

    try {
        socketCliente = new Socket('s'); 
        if (socketCliente->MakeConnection(this->IP, this->puerto)) { 
            std::cerr << "Fallo en conexión TCP" << std::endl;
            return -1;
        }
        std::cout << "Conexión establecida con el servidor." << std::endl;
        this->listFiles();

    } catch (const std::exception& e) {
        std::cerr << "Excepción al conectar TCP: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

/*void Cliente::sendRequest(){
    const char* request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
    socketCliente->Write(request);
    socketCliente->Read(buffer, 512);
    this->showResponse();
}*/

void Cliente::sendProtocolRequest(const std::string& command, const std::string& filename, const std::string& content) {
    std::string request;
    
    if (command == "list") {
        request = "list Figures\r\n";
    } else if (command == "request") {
        request = "request " + filename + "\r\n";
    } else if (command == "delete") {
        request = "delete " + filename + "\r\n";
    }

    if (!socketCliente) {
    std::cerr << "Error: socket no inicializado" << std::endl;
    return;
   }
    
    socketCliente->Write(request.c_str());
    socketCliente->Read(buffer, 512);
    this->showResponse();
}


/*int Cliente::connectSSL() {
    this->puerto= 443;
    std::cout << "Conectando a " << IP << ":" << puerto << " usando SSL..." << std::endl;
    try {
        socketCliente = new SSLSocket(); 
        if (socketCliente->MakeConnection(this->IP, this->puerto)) { 
            std::cerr << "Fallo en conexión SSL" << std::endl;
            return -1;
        }
        std::cout << "Conexión establecida con el servidor." << std::endl;

        this->sendRequest();
    } catch (const std::exception& e) {
        std::cerr << "Excepción al conectar SSL: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}*/

void Cliente::showResponse() {
    std::string contenido;
    const char* inicio = strstr(buffer, "<PRE>");
    if (inicio) {
        inicio += 5; // Saltar "<PRE>"
        const char* fin = strstr(inicio, "</PRE>");
        if (fin) {
            contenido.assign(inicio, fin - inicio);
        } else {
            contenido.assign(inicio); // Si no hay </PRE>, tomar hasta el final
        }
    } else {
        contenido.assign(buffer); // Si no hay <PRE>, tomar todo
    }
    std::cout << contenido << std::endl;
}


void Cliente::listFiles() {
    this->sendProtocolRequest("list");
}

void Cliente::requestFile(const char* filename) {
    this->sendProtocolRequest("request", filename);
}

void Cliente::deleteFile(const char* filename) {
    this->sendProtocolRequest("delete", filename);
}

void Cliente::submitFile(const char* filename, const char* content) {
    // Para submit es especial porque envía comando + contenido
    std::string request = "submit " + std::string(filename) + "\r\n";
    socketCliente->Write(request.c_str());
    
    // Enviar el contenido del archivo
    socketCliente->Write(content);
    
    socketCliente->Read(buffer, 512);
    this->showResponse();
}
