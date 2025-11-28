#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "SSLSocket.h"
#include "VSocket.h"

#define DISCOVERY_PORT 9000
#define MAXBUF         2048

struct DrawingServerInfo {
    std::string ip;
    int         port = 0;
    std::set<std::string> figuras;
};

using ServerKey = std::string; // ip:puerto

// ------------------ utilidades ------------------

ServerKey makeKey(const std::string& ip, int port) {
    return ip + ":" + std::to_string(port);
}

VSocket* connectToDrawingServer(const std::string& ip, int port) {
    try {
        std::cout << "[TENEDOR] Conectando a " << ip << ":" << port 
                  << " como cliente SSL..." << std::endl;

        // Cliente SSL usa constructor sin certificados
        VSocket* client = new SSLSocket(false, false);
        client->MakeConnection(ip.c_str(), port);
        
        std::cout << "[TENEDOR] Conexión SSL establecida correctamente" << std::endl;
        return client;
        
    } catch (const std::exception& e) {
        std::cerr << "[TENEDOR] Error en conexión SSL: " << e.what() << std::endl;
        return nullptr;
    }
}

std::string extractBodyFromResponse(const std::string& httpResponse) {
    size_t headerEnd = httpResponse.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return httpResponse;
    }
    std::string body = httpResponse.substr(headerEnd + 4);
    size_t preStart = body.find("<pre>");
    size_t preEnd   = body.find("</pre>");
    if (preStart != std::string::npos && preEnd != std::string::npos) {
        return body.substr(preStart + 5, preEnd - (preStart + 5));
    }
    return body;
}

std::set<std::string> parseFigureList(const std::string& body) {
    std::set<std::string> figuras;
    std::istringstream iss(body);
    std::string line;

    while (std::getline(iss, line)) {
        // quita espacios al inicio y final
        while (!line.empty() && std::isspace((unsigned char)line.front()))
            line.erase(line.begin());
        while (!line.empty() && std::isspace((unsigned char)line.back()))
            line.pop_back();

        if (!line.empty()) {
            figuras.insert(line);
        }
    }
    return figuras;
}

void updateFigureListForServer(DrawingServerInfo& info) {
    // Body XML
    std::string xmlBody =
        "<Body>\n"
        "    <Action>List</Action>\n"
        "    <Figure>*</Figure>\n"
        "    <Content></Content>\n"
        "</Body>";

    std::string httpRequest =
        "POST / HTTP/1.1\r\n"
        "Host: " + info.ip + "\r\n"
        "Content-Type: application/xml\r\n"
        "Content-Length: " + std::to_string(xmlBody.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        xmlBody;

    std::cout << "[TENEDOR] Pidiendo lista a "
              << info.ip << ":" << info.port << std::endl;

    VSocket* client = connectToDrawingServer(info.ip, info.port);
    
    // Verificar si la conexión falló
    if (client == nullptr) {
        std::cerr << "[TENEDOR] No se pudo conectar al servidor de dibujo" << std::endl;
        return;
    }

    std::string response;
    char buf[MAXBUF];
    int bytes = 0;

    try {
        // 1) Enviar request
        client->Write(httpRequest.c_str(), httpRequest.size());

        // 2) Leer headers
        while ((bytes = client->Read(buf, sizeof(buf))) > 0) {
            response.append(buf, bytes);
            size_t headerEnd = response.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // Ya tenemos todos los headers
                // 3) Buscar Content-Length
                size_t clPos = response.find("Content-Length:");
                int contentLength = 0;
                if (clPos != std::string::npos) {
                    size_t start = clPos + 15;
                    size_t end   = response.find("\r\n", start);
                    std::string lengthStr = response.substr(start, end - start);
                    contentLength = std::stoi(lengthStr);
                }

                // 4) Leer solo el cuerpo que falta
                size_t bodyStart = headerEnd + 4;
                int bodyReceived = response.size() - bodyStart;
                int remaining    = contentLength - bodyReceived;

                while (remaining > 0) {
                    int toRead = std::min(remaining, (int)sizeof(buf));
                    bytes = client->Read(buf, toRead);
                    if (bytes <= 0) break;
                    response.append(buf, bytes);
                    remaining -= bytes;
                }

                break; // ya no leemos mas
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[TENEDOR] Error leyendo respuesta SSL: "
                  << e.what() << std::endl;
    }

    // Cerrar y eliminar el cliente de forma segura
    if (client) {
        client->Close();
        delete client;
    }

    std::string body = extractBodyFromResponse(response);
    info.figuras = parseFigureList(body);

    std::cout << "[TENEDOR] Figuras en " << info.ip << ":" << info.port << std::endl;
    for (const auto& f : info.figuras) {
        std::cout << "   - " << f << std::endl;
    }
}

// "HELLO_DRAWING;PORT=8080"
bool parseHelloMessage(const std::string& msg, int& portOut) {
    const std::string prefix = "HELLO_DRAWING";
    if (msg.rfind(prefix, 0) != 0) {
        return false;
    }
    std::size_t pos = msg.find("PORT=");
    if (pos == std::string::npos) {
        return false;
    }
    std::string portStr = msg.substr(pos + 5);
    portOut = std::atoi(portStr.c_str());
    return (portOut > 0);
}

// ------------------ main ------------------

int main(int argc, char** argv) {
    int udpPort = DISCOVERY_PORT;
    if (argc > 1) {
        udpPort = std::atoi(argv[1]);
    }

    std::cout << "[TENEDOR] Escuchando HELLO UDP en puerto " << udpPort << std::endl;

    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) {
        perror("[TENEDOR] socket UDP");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(udpPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udpSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("[TENEDOR] bind UDP");
        close(udpSock);
        return 1;
    }

    // tabla de rutas: key = "ip:puerto"
    std::map<ServerKey, DrawingServerInfo> servers;

    while (true) {
        char buffer[MAXBUF];
        sockaddr_in from{};
        socklen_t fromLen = sizeof(from);

        int n = recvfrom(udpSock, buffer, sizeof(buffer) - 1, 0,
                         reinterpret_cast<sockaddr*>(&from), &fromLen);
        if (n < 0) {
            perror("[TENEDOR] recvfrom");
            continue;
        }

        buffer[n] = '\0';
        std::string msg(buffer);
        std::string ip = inet_ntoa(from.sin_addr);

        std::cout << "[TENEDOR] UDP desde " << ip
                  << ": \"" << msg << "\"" << std::endl;

        int port = 0;
        if (!parseHelloMessage(msg, port)) {
            std::cout << "[TENEDOR] Mensaje ignorado (no es HELLO_DRAWING)" << std::endl;
            continue;
        }

        ServerKey key = makeKey(ip, port);
        auto it = servers.find(key);
        if (it == servers.end()) {
            // servidor nuevo
            DrawingServerInfo info;
            info.ip   = ip;
            info.port = port;

            std::cout << "[TENEDOR] Nuevo servidor de figuras: " << key << std::endl;
            updateFigureListForServer(info);
            servers[key] = info;
        } else {
            std::cout << "[TENEDOR] HELLO de servidor conocido: " << key << std::endl;
            updateFigureListForServer(it->second);
        }

        // Debug: imprimir tabla de rutas
        std::cout << "[TENEDOR] Tabla de rutas actual:" << std::endl;
        for (const auto& [k, info] : servers) {
            std::cout << " Servidor " << k << " tiene figuras:" << std::endl;
            for (const auto& f : info.figuras) {
                std::cout << "   * " << f << std::endl;
            }
        }
    }

    close(udpSock);
    return 0;
}