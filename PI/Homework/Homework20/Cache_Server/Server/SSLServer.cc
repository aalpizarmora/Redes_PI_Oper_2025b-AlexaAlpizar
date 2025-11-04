#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <thread>
#include <sstream>

#include "SSLSocket.h"
#include "CacheManager.h"

#define PORTFS 8082 // Puerto que se utilizará para conectar con el FileSystem
#define DRAWING_SERVER_PORT 8080

/**
 * @brief Extrae el cuerpo de la respuesta HTTP
 */
std::string extractBodyFromResponse(const std::string& httpResponse) {
    // Buscar el fin de los headers HTTP
    size_t headerEnd = httpResponse.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return "Error: Invalid HTTP response";
    }
    
    std::string body = httpResponse.substr(headerEnd + 4);
    
    // Extraer contenido entre <pre> tags (formato que usa el cliente)
    size_t preStart = body.find("<pre>");
    size_t preEnd = body.find("</pre>");
    
    if (preStart != std::string::npos && preEnd != std::string::npos) {
        return body.substr(preStart + 5, preEnd - (preStart + 5));
    }
    
    // Si no hay tags <pre>, devolver el cuerpo completo
    return body;
}

/**
 * @brief Función integrada para comunicarse con el Servidor de Dibujos
 */
std::string fetchFromDrawingServer(VSocket* client, const std::string& figureName) {
    try {
        std::cout << "Connecting to drawing server for figure: " << figureName << std::endl;
        
        // // // Crear socket como CLIENTE para conectar al servidor de dibujos
        // SSLSocket drawingServer(false, false);  // server = false (modo cliente)
        
        // // // Conectar al servidor de dibujos
        // drawingServer.MakeConnection("127.0.0.1", DRAWING_SERVER_PORT);
        
        std::cout << "Connected to drawing server, sending request..." << std::endl;
        
        // Construir request
        std::string xmlRequest = 
            "<Body>\n"
            "    <Action>MISS</Action>\n"
            "    <Figure>" + figureName + "</Figure>\n"
            "    <Content></Content>\n"
            "</Body>";
        
        // Construir request HTTP 
        std::string httpRequest = 
            "POST / HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Type: application/xml\r\n"
            "Content-Length: " + std::to_string(xmlRequest.size()) + "\r\n"
            "\r\n" +
            xmlRequest;
        
        // Envar request al servidor de dibujos
        client->Write(httpRequest.c_str(), httpRequest.size());
        
        std::cout << "Request sent, waiting for response..." << std::endl;
        
        // Recibir respuesta
        std::string response;
        char buffer[1024];
        int bytes;
        
        while ((bytes = client->Read(buffer, sizeof(buffer))) > 0) {
            response.append(buffer, bytes);
            if (response.find("\r\n\r\n") != std::string::npos) break;
        }

        std::cout << "Received response from drawing server, size: " << response.size() << " bytes" << std::endl;
        
        // Extraer el contenido de la respuesta
        std::string content = extractBodyFromResponse(response);
        
        if (!content.empty() && content.find("Error") == std::string::npos) {
            std::cout << "Successfully retrieved figure from drawing server" << std::endl;
            return content;
        } else {
            std::cerr << "No content extracted from drawing server response" << std::endl;
            return "Error: Figure not found in drawing server";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error connecting to drawing server: " << e.what() << std::endl;
        return "Error: Cannot connect to drawing server";
    }
}


/**
 * @brief Procesa requests HTTP para el servidor cache
 */
void processResponse(VSocket* client, CacheManager* cache) {  
    std::string request;
    char buf[1024];
    int bytes;

    while ((bytes = client->Read(buf, sizeof(buf))) > 0) {
        request.append(buf, bytes);
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    size_t pos = request.find("\r\n\r\n");
    std::string body = (pos != std::string::npos) ? request.substr(pos + 4) : request;

    std::string action, figure, content;
    
    size_t actionStart = body.find("<Action>");
    size_t actionEnd = body.find("</Action>");
    if (actionStart != std::string::npos && actionEnd != std::string::npos) {
        action = body.substr(actionStart + 8, actionEnd - (actionStart + 8));
    }

    size_t figureStart = body.find("<Figure>");
    size_t figureEnd = body.find("</Figure>");
    if (figureStart != std::string::npos && figureEnd != std::string::npos) {
        figure = body.substr(figureStart + 8, figureEnd - (figureStart + 8));
    }

    size_t contentStart = body.find("<Content>");
    size_t contentEnd = body.find("</Content>");
    if (contentStart != std::string::npos && contentEnd != std::string::npos) {
        content = body.substr(contentStart + 9, contentEnd - (contentStart + 9));
    }

    std::cout << "Cache Server - Action: '" << action << "' Figure: '" << figure << "'" << std::endl;

    std::string responseData;
    bool success = true;

    if (action == "Request" || action == "RequestCache") {
        // Buscar en cache
        if (cache->contains(figure)) {
            const char * requestMessageSimple
                = "\n<Body>\n\
                        \t<Action>%s</Action>\n\
                        \t<Figure>%s</Figure>\n\
                        \t<Content>%s</Content>\n\
                        </Body>\n";

            std::cout << "Cache HIT for: " << figure << std::endl;
            responseData = cache->retrieve(figure);
            size_t buffSize = responseData.size() + 512;
            char* dynamicRequest = new char[buffSize];
            snprintf( dynamicRequest, buffSize, requestMessageSimple, "HIT", figure.c_str(), responseData.c_str() );
            responseData = dynamicRequest;
            std::cout<< responseData << std::endl;
        } else {
            // Si no está en cache, buscar en servidor de dibujos
            std::cout << "Cache MISS for: " << figure << " - Fetching from drawing server..." << std::endl;
            responseData = fetchFromDrawingServer(client, figure);  //  USAR función libre
            std::cout<< responseData << std::endl;
            // Guardar en cache (con política de reemplazo)
            if (!responseData.empty() && responseData.find("Error") == std::string::npos) {
                cache->store(figure, responseData);
                std::cout << "Stored in cache: " << figure << std::endl;
            }
        }
    } 
    else if (action == "Submit") {
        // Almacenar directamente en cache
        cache->store(figure, content);
        responseData = "Stored in cache: " + figure;
    }
    else if (action == "Delete") {
        // Invalidar entrada de cache
        cache->invalidate(figure);
        responseData = "Invalidated from cache: " + figure;
    }
    else if (action == "List") {
        // Listar claves en cache
        responseData = cache->listKeys();
    }
    else {
        success = false;
        responseData = "Unknown action";
    }

    // Construir respuesta HTTP
    std::string response;
    if (success) {
        std::string bodyContent = "<html><body><pre>\n" + responseData + "\n</pre></body></html>\n";
        response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(bodyContent.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + bodyContent;
    } else {
        response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><pre>" + responseData + "</pre></body></html>";
    }

    client->Write(response.c_str(), response.size());
}

/**
 * @brief Función de servicio para el servidor cache
 */
void Service(VSocket* client, CacheManager* cache) {
    try {
        if (auto sslc = dynamic_cast<SSLSocket*>(client)) {
            sslc->AcceptSSL();  // <-- asegúrate de llamar el método que ejecuta SSL_accept()
        }
        processResponse(client, cache);
    } catch (const std::exception& e) {
        std::cerr << "Error in cache service: " << e.what() << std::endl;
    }
    client->Close();
}

/**
 * @brief Función principal del servidor
 */
int main(int cuantos, char** argumentos) {
    // Inicializar cache
    CacheManager* cache = new CacheManager();
    
    std::cout << "Cache Server started - Max size: 4KB, Line size: 1KB" << std::endl;

    VSocket* server;
    std::thread* worker;
    int port = PORTFS;
    
    if (cuantos > 1) {
        port = atoi(argumentos[1]);
    }

    // USAR SSLSocket para comunicación segura
    #ifdef USE_IPV6
        server = new SSLSocket("Server/ci0123.pem", "Server/key0123.pem", true, true);
    #else
        server = new SSLSocket("Server/ci0123.pem", "Server/key0123.pem", false, true);
    #endif

    try {
        server->Bind(port);
        server->MarkPassive(10);
        std::cout << "Cache Server listening on port " << port << std::endl;

        for(;;) {
            VSocket* client = server->AcceptConnection();
            std::cout << "New connection accepted" << std::endl;
            
            // Pasar solo cache al thread
            worker = new std::thread(Service, client, cache);
            worker->detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    delete cache;
    delete server;
    return 0;
}