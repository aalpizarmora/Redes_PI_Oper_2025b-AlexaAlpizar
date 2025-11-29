/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2025-i
 *  Grupo 6
 *
 *  ForkServer.cc - Servidor coordinador entre múltiples servidores y clientes
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <regex>

#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Socket.h"

#define MAXBUF 256
#define PORT_CLIENT 8080    // Puerto para clientes
#define PORT_SERVER 8081    // Puerto para servidores
#define PORT_UDP 4321       // Puerto UDP para señalización

/**
 * Estructura para almacenar información de servidores
 */
struct ServerInfo {
  std::string ip;
  std::string hostname;
  int port;
  // bool active;
  std::vector<std::string> figures;
};

/**
 * Clase Tenedor - Coordinador del sistema distribuido
 */
class Tenedor {
private:
  std::map<std::string, ServerInfo> servers;  // IP -> ServerInfo
  std::map<std::string, std::string> figureLocation;  // figura -> IP servidor
  std::mutex serversMutex;
  VSocket* clientListener;
  VSocket* udpSocket;
  int currentServerIndex;

public:
  Tenedor() : currentServerIndex(0) {
    clientListener = nullptr;
    udpSocket = nullptr;
  }

  ~Tenedor() {
    if (clientListener) {
      clientListener->Close();
      delete clientListener;
    }
    if (udpSocket) {
      udpSocket->Close();
      delete udpSocket;
    }
  }

  /**
   * Parsea un mensaje con formato: COMANDO /BEGIN/ [params] /END/
   */
  bool parseMessage(const std::string& message, std::string& command, 
                    std::vector<std::string>& params, std::string& postscript) {
    std::regex cmdRegex(
        "([A-Z]+)\\s*/BEGIN/\\s*(.*?)\\s*/END/\\s*([\\s\\S]*)"
    );

    std::smatch matches;
    
    if (std::regex_search(message, matches, cmdRegex)) {
      command = matches[1].str();
      std::string paramsStr = matches[2].str();
      postscript = matches[3].str();

      postscript.erase(0, postscript.find_first_not_of(" \t\r\n"));
      postscript.erase(postscript.find_last_not_of(" \t\r\n") + 1);
      
      // Separar parámetros por '/'
      size_t pos = 0;
      while (pos < paramsStr.length()) {
        size_t nextSlash = paramsStr.find('/', pos);
        if (nextSlash == std::string::npos) {
          std::string param = paramsStr.substr(pos);
          param.erase(0, param.find_first_not_of(" \t\r\n"));
          param.erase(param.find_last_not_of(" \t\r\n") + 1);
          if (!param.empty() && param != " ") {
            params.push_back(param);
          }
          break;
        }
        std::string param = paramsStr.substr(pos, nextSlash - pos);
        param.erase(0, param.find_first_not_of(" \t\r\n"));
        param.erase(param.find_last_not_of(" \t\r\n") + 1);
        if (!param.empty() && param != " ") {
          params.push_back(param);
        }
        pos = nextSlash + 1;
      }
      return true;
    }
    return false;
  }

  std::string buildXML(const std::string& action,
                     const std::string& figure,
                     const std::string& content) 
  {
    printf("Building XML with action: %s, figure: %s, content: %s\n",
           action.c_str(), figure.c_str(), content.c_str());  
    return "<Body>\n"
        "\t<Action>" + action + "</Action>\n"
        "\t<Figure>" + figure + "</Figure>\n"
        "\t<Content>" + content + "</Content>\n"
        "</Body>\n";
  }

  /**
   * Construye un mensaje de respuesta
   */
  std::string buildResponse(int code, const std::string& data) {
    std::string status;
    switch(code) {
      case 200: status = "200 OK"; break;
      case 400: status = "400 Bad Request"; break;
      case 500: status = "500 Internal Server Error"; break;
      default: status = "500 Internal Server Error";
    }
    
    std::string response = status + " /BEGIN/ " + data + " /END/\r\n";
    return response;
  }

  /**
   * Comando LIST - Retorna lista de todas las figuras
   */
  std::string handleList() {
    std::lock_guard<std::mutex> lock(serversMutex);
    std::string result;
    
    for (const auto& entry : figureLocation) {
      result += entry.first + "\n";
    }

    if (result.empty()) {
      result = "No hay figuras disponibles";
    }
    
    return buildResponse(200, result);
  }

  /**
   * Comando GET - Obtiene una figura específica
   */
  std::string handleGet(const std::string& figureName) {
    std::lock_guard<std::mutex> lock(serversMutex);
    
    auto it = figureLocation.find(figureName);
    if (it == figureLocation.end()) {
      return buildResponse(400, "Figura no encontrada");
    }
    
    std::string serverIP = it->second;
    auto serverIt = servers.find(serverIP);
    
    if (serverIt == servers.end()) {
      return buildResponse(500, "Servidor no disponible");
    }
    
    try {
      VSocket* serverConn = new Socket('s', false);
      serverConn->MakeConnection(serverIP.c_str(), PORT_SERVER);
      
      std::string xml = buildXML("Request", figureName, "");
      std::string request = 
          "POST / HTTP/1.1\r\n"
          "Content-Type: application/xml\r\n"
          "Content-Length: " + std::to_string(xml.size()) + "\r\n"
          "\r\n" +
          xml;

      serverConn->Write(request.c_str(), request.length());
      
      char buffer[MAXBUF];
      std::string response;
      int bytes;
      
      while ((bytes = serverConn->Read(buffer, sizeof(buffer))) > 0) {
        response.append(buffer, bytes);
        if (response.find("/END/") != std::string::npos) break;
      }
      
      serverConn->Close();
      delete serverConn;
      
      return response;
    } catch (...) {
      return buildResponse(500, "Error de comunicación con servidor");
    }
  }

  /**
   * Comando ADD - Agrega una nueva figura al sistema
   */
  std::string handleAdd(const std::string& figureName, const std::string& size, 
                        const std::string& content) {
    std::lock_guard<std::mutex> lock(serversMutex);
    if (figureLocation.find(figureName) != figureLocation.end()) {
      return buildResponse(400, "Figura ya existe");
    }
    
    std::string selectedServer;
    int count = 0;
    int activeServers = 0;
    for (const auto& server : servers) {
      activeServers++;
    }

    if (activeServers == 0)
      return buildResponse(500, "No hay servidores disponibles");

    for (const auto& server : servers) {
      if (count == currentServerIndex % activeServers) {
        selectedServer = server.first;
        break;
      }
      count++;
    }
    
    currentServerIndex++;
    
    try {
      VSocket* serverConn = new Socket('s', false);
      serverConn->MakeConnection(selectedServer.c_str(), PORT_SERVER);
      std::string trimmedContent
       = content.substr(0, std::min((size_t)stoi(size), content.size()));
      printf("Contenido a enviar (tamaño %zu): %s\n", trimmedContent.size(), trimmedContent.c_str());
      std::string xml = buildXML("Submit", figureName, trimmedContent);
      std::string request =
          "POST / HTTP/1.1\r\n"
          "Content-Type: application/xml\r\n"
          "Content-Length: " + std::to_string(xml.size()) + "\r\n\r\n" +
          xml;

      serverConn->Write(request.c_str(), request.length());
      
      char buffer[MAXBUF];
      std::string response;
      int bytes;
      
      while ((bytes = serverConn->Read(buffer, sizeof(buffer))) > 0) {
        response.append(buffer, bytes);
        if (response.find("/END/") != std::string::npos) break;
      }
      
      serverConn->Close();
      delete serverConn;
      
      figureLocation[figureName] = selectedServer;
      servers[selectedServer].figures.push_back(figureName);
      
      return buildResponse(200, "Figura agregada exitosamente");
    } catch (...) {
      return buildResponse(500, "Error al agregar figura");
    }
  }

  /**
   * Comando DELETE - Elimina una figura del sistema
   */
  std::string handleDelete(const std::string& figureName) {
    std::lock_guard<std::mutex> lock(serversMutex);
    auto it = figureLocation.find(figureName);
    if (it == figureLocation.end()) {
      return buildResponse(400, "Figura no encontrada");
    }
    
    std::string serverIP = it->second;
    
    try {
      VSocket* serverConn = new Socket('s', false);
      serverConn->MakeConnection(serverIP.c_str(), PORT_SERVER);
      
      std::string xml = buildXML("Delete", figureName, "");
      std::string request =
          "POST / HTTP/1.1\r\n"
          "Content-Type: application/xml\r\n"
          "Content-Length: " + std::to_string(xml.size()) + "\r\n\r\n" +
          xml;

      serverConn->Write(request.c_str(), request.length());
      
      char buffer[MAXBUF];
      std::string response;
      int bytes;
      
      while ((bytes = serverConn->Read(buffer, sizeof(buffer))) > 0) {
        response.append(buffer, bytes);
        if (response.find("/END/") != std::string::npos) break;
      }
      
      serverConn->Close();
      delete serverConn;
      
      figureLocation.erase(figureName);
      auto& figures = servers[serverIP].figures;
      figures.erase(std::remove(figures.begin(), figures.end(), figureName), 
                    figures.end());
      
      return buildResponse(200, "Figura eliminada exitosamente");
    } catch (...) {
      return buildResponse(500, "Error al eliminar figura");
    }
  }

  /**
   * Procesa solicitudes de clientes
   */
  void processClientRequest(VSocket* client) {
    char buffer[MAXBUF];
    std::string rawRequest;
    int bytes;

    int contentLength = -1;
    size_t headerEndPos = std::string::npos;
    
    try {
      // Leer TODO lo que el cliente envíe hasta que cierre la conexión
      while ((bytes = client->Read(buffer, sizeof(buffer))) > 0) {
            rawRequest.append(buffer, bytes);

            // 1) ¿Ya tenemos los headers completos?
            if (headerEndPos == std::string::npos) {
                headerEndPos = rawRequest.find("\r\n\r\n");
                if (headerEndPos != std::string::npos) {
                    // Extraer headers
                    std::string headers = rawRequest.substr(0, headerEndPos);

                    // Buscar Content-Length (insensible a espacios)
                    std::string clKey = "Content-Length:";
                    size_t pos = headers.find(clKey);
                    if (pos != std::string::npos) {
                        pos += clKey.size();
                        // Saltar espacios
                        while (pos < headers.size() && isspace((unsigned char)headers[pos])) {
                            ++pos;
                        }
                        size_t endPos = pos;
                        while (endPos < headers.size() && isdigit((unsigned char)headers[endPos])) {
                            ++endPos;
                        }
                        std::string clStr = headers.substr(pos, endPos - pos);
                        contentLength = std::stoi(clStr);
                        // std::cout << "Content-Length detectado: " << contentLength << std::endl;
                    }
                }
            }

            // 2) Si ya sabemos el Content-Length, verificar si ya llegó TODO el body
            if (contentLength != -1 && headerEndPos != std::string::npos) {
                size_t bodyStart = headerEndPos + 4; // "\r\n\r\n"
                size_t bodySize  = rawRequest.size() - bodyStart;
                if ((int)bodySize >= contentLength) {
                    // Ya tenemos todo el cuerpo, no es necesario leer más
                    break;
                }
            }

            // Si todavía no tenemos Content-Length, seguimos leyendo
            // hasta que aparezcan headers completos.
        }
      
      std::cout << "Solicitud recibida: " << rawRequest << std::endl;
      
      std::string command;
      std::vector<std::string> params;
      std::string postscript;
      if (!parseMessage(rawRequest, command, params, postscript)) {
        std::string response = buildResponse(400, "Formato de comando incorrecto");
        client->Write(response.c_str(), response.length());
        client->Close();
        delete client;
        return;
      }
      
      std::string response;
      if (command == "LIST") {
        response = handleList();
      } else if (command == "GET" && params.size() >= 1) {
        response = handleGet(params[0]);
      } else if (command == "ADD" && params.size() >= 2) {
        response = handleAdd(params[0], params[1], postscript);
      } else if (command == "DELETE" && params.size() >= 1) {
        response = handleDelete(params[0]);
      } else {
        response = buildResponse(400, "Comando no reconocido o parámetros faltantes");
      }
      
      client->Write(response.c_str(), response.length());
    } catch (const std::exception& e) {
      std::cerr << "Error procesando solicitud: " << e.what() << std::endl;
    }
    client->Close();
    delete client;
  }

  /**
   * Maneja señalización UDP de servidores
   */
  void handleUDPSignaling() {
    try {
      udpSocket = new Socket('d', false);
      udpSocket->Bind(PORT_UDP);
      
      std::string buffer(MAXBUF, '\0');
      struct sockaddr_in clientAddr;
      
      std::cout << "Escuchando señalización UDP en puerto " << PORT_UDP << std::endl;
      
      while (true) {
        
        memset(&clientAddr, 0, sizeof(clientAddr));
        
        size_t bytes = udpSocket->recvFrom(
            &buffer[0], 
            buffer.size() - 1,
            &clientAddr
        );

        if (bytes > 0) {
          buffer[bytes] = '\0';
          std::string message(buffer);
          
          char senderIP[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &(clientAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
          
          std::cout << "UDP recibido de " << senderIP << ": " << message << std::endl;
          
          std::string command;
          std::vector<std::string> params;
          std::string postscript;
          
          if (parseMessage(message, command, params, postscript) && params.size() > 0) {
            std::string ip = params[0];
            
            std::lock_guard<std::mutex> lock(serversMutex);
            std::string hostname;
            
            if (command == "CONNECT") {
              std::cout << "Servidor conectado: " << ip << std::endl;
              ServerInfo info;
              info.ip = ip;
              info.hostname = hostname;
              info.port = PORT_SERVER;

              servers[ip] = info;

              std::vector<std::string> figures;

              std::string xmlMsg = buildXML("List", "Figures", "");
              const char* requestMessageSimple = xmlMsg.c_str();
                
              // ----------- OBTENER FIGURAS DEL SERVIDOR ------------
              figures = getFiguresFromServer(ip.c_str(), PORT_SERVER, requestMessageSimple);

              // Registrar ubicación de figuras
              for (auto &f : figures) {
                  figureLocation[f] = ip;
              }

              servers[ip].figures = figures;

              printf("Server %s (%s:%d) registrado con %zu figuras.\n",
                    hostname.c_str(), ip.c_str(), PORT_SERVER, servers[ip].figures.size());
            } else if (command == "QUIT") {
              std::cout << "Servidor desconectado: " << ip << std::endl;
              if (servers.find(ip) != servers.end()) {
                
                for (const auto& fig : servers[ip].figures) {
                  figureLocation.erase(fig);
                }
              }
            }
          }
        }
      }
    } catch (const std::exception& e) {
      std::cerr << "Error en señalización UDP: " << e.what() << std::endl;
    }
  }

  /**
   * Inicia el servidor tenedor
   */
  void start() {
    // broadcast();

    std::thread udpThread(&Tenedor::handleUDPSignaling, this);
    udpThread.detach();
    
    clientListener = new Socket('s', false);
    clientListener->Bind(PORT_CLIENT);
    clientListener->MarkPassive(10);
    
    std::cout << "========================================" << std::endl;
    std::cout << "                TENEDOR                 " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Puerto clientes (TCP): " << PORT_CLIENT << std::endl;
    std::cout << "Puerto servidores (TCP): " << PORT_SERVER << std::endl;
    std::cout << "Puerto señalización (UDP): " << PORT_UDP << std::endl;
    std::cout << "Esperando conexiones..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    while (true) {
      try {
        VSocket* client = clientListener->AcceptConnection();
        std::thread* worker = new std::thread(&Tenedor::processClientRequest, 
                                              this, client);
        worker->detach();
      } catch (const std::exception& e) {
        std::cerr << "Error aceptando conexión: " << e.what() << std::endl;
      }
    }
  }

  /**
   * @brief Creates a connection to the remote server
   * It creates an HTTP or HTTPS socket depending on arguments, and establishes
   * a connection to the remote server.
   *
   * @param argc Argument count
   *  - If argc > 1: HTTPS connection
   *  - If argc == 1: HTTP connection
   * @param os Remote server IP
   *
   * @return VSocket* Pointer to the created socket
  **/
  VSocket* connectToServer(const char* hostname, int port) {
    VSocket* client;
    #ifdef USE_IPV6
      // IPv6
      client = new Socket('s', true);
    #else
      // IPv4
      client = new Socket('s', false);
    #endif
    client->MakeConnection( hostname, port );
    return client;
  }

  std::string receiveFiguresList(char* clientRequest, VSocket* client) {
    // construir el request HTTP
    std::string requestStr(clientRequest);
    std::string request = 
        "POST / HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/xml\r\n"
        "Content-Length: " + std::to_string(requestStr.size()) + "\r\n"
        "\r\n" +
        requestStr;

    client->Write(request.c_str(), request.size());

    std::string response;
    char buf[MAXBUF];
    int bytes;

    // Ciclo para leer la respuesta completa del server
    while ((bytes = client->Read(buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        response += buf;
    }

    size_t start = response.find("<pre>");
    size_t end   = response.find("</pre>");
    std::string art;
    if (start != std::string::npos && end != std::string::npos) {
        art = response.substr(start + 6, end - (start + 6));
    } else {
        art = response;
    }

    while (!art.empty() && std::isspace((unsigned char)art.back())) {
      art.pop_back();
    }

    // Se muestra el resultado de la consulta al server
    // printf("Bytes read total: %zu\n%s\n", response.size(), art.c_str());

    return art;
  }

  std::vector<std::string> getFiguresFromServer(const char* serverIP, int serverPort, const char* requestMessageSimple) {
    VSocket * client = connectToServer(serverIP, serverPort);
    char clientRequest[ 1024 ] = { 0 };
    sprintf( clientRequest, requestMessageSimple );
    std::string responseReceived = receiveFiguresList(clientRequest, client);
    client->Close();
    client = nullptr;

    // Delimiter
    std::string del = ",";

    // Find first occurrence of the delimiter
    auto pos = responseReceived.find(del);

    std::vector<std::string> currentServerInfo;

    // While there are still delimiters in the
    // string
    while (pos != std::string::npos) {

        // Extracting the substring up to the
        // delimiter
        if (responseReceived.substr(0, pos) != "") {
          // std::cout << "\"" << responseReceived.substr(0, pos) <<
          //   "\"" << " ";
          currentServerInfo.push_back(responseReceived.substr(0, pos));
        }
        // Erase the extracted part from the
        // original string
        responseReceived.erase(0, pos + del.length());

        // Find the next occurrence of the
        // delimiter
        pos = responseReceived.find(del);
    }
      // Output the last substring (after the last
    // delimiter)
    if (responseReceived != "") {
      // std::cout << "\"" << responseReceived << "\"" << " " << std::endl;
      currentServerInfo.push_back(responseReceived);
    }

    return currentServerInfo;
  }
};

int main(int argc, char** argv) {
  Tenedor tenedor;
  tenedor.start();
  return 0;
}