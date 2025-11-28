/**
*  Universidad de Costa Rica
*  ECCI
*  CI0123 Proyecto integrador de redes y sistemas operativos
*  2025-i
*  Grupos: 1 y 3
*/

#include <cstdio>   // printf
#include <cstdlib>  // atoi
#include <cstring>  // strlen, strcmp
#include <iostream>
#include <regex>
#include <thread>
#include <sstream>

// UDP para avisar al Tenedor
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Socket.h"
#include "SSLSocket.h"
#include "FileSystem.h"

#define MAXBUF 256
#define PORTCLIENT 8080
#define MAX_FILESIZE 132096  // 4 * 256 + 256 * 256 * 2

// Datos del Tenedor
#define TENEDOR_IP       "127.0.0.1"
#define TENEDOR_UDP_PORT 9000

/**
 * @brief Envía un aviso al TENEDOR por UDP indicando el puerto HTTPS/TCP
 *        de este servidor de figuras, usando la clase Socket (UDP).
 *
 * Formato del mensaje:
 *   HELLO_DRAWING;PORT=<puerto>
 */
void notifyTenedor(int drawingPort) {
  try {
    // 'd' = datagram (UDP), false = IPv4
    Socket udp('d', false);

    // Conectamos el socket UDP al Tenedor
    udp.MakeConnection(TENEDOR_IP, TENEDOR_UDP_PORT);

    std::string message = "HELLO_DRAWING;PORT=" + std::to_string(drawingPort);

    // Enviamos el datagrama
    udp.Write(message.c_str(), message.size());
    std::cout << "[DRAWING] Enviado HELLO al Tenedor (" << TENEDOR_IP
              << ":" << TENEDOR_UDP_PORT << "): " << message << std::endl;

    // Cerramos el socket UDP
    udp.Close();

    //Si hay error, se captura la excepcion
  } catch (const std::exception& e) {
    std::cerr << "[DRAWING] Error al notificar al Tenedor vía UDP: "
              << e.what() << std::endl;
  }
}

/**
 * @brief Procesa un request HTTP y genera la respuesta.
 *
 * Soporta acciones:
 *   - List:    lista todo el contenido del FileSystem
 *   - Request: devuelve una figura específica
 *   - Submit:  crea/escribe figura
 *   - Delete:  elimina figura
 */
void processResponse(VSocket* client, FileSystem* fs) {
  std::string request;
  char buf[MAXBUF];
  int bytes;

  // Leer headers
  while ((bytes = client->Read(buf, sizeof(buf))) > 0) {
    request.append(buf, bytes);
    if (request.find("\r\n\r\n") != std::string::npos) break;
  }

  // Leer cuerpo si tiene Content-Length
  size_t contentLengthPos = request.find("Content-Length:");
  int contentLength = 0;
  if (contentLengthPos != std::string::npos) {
    size_t start = contentLengthPos + 15;
    size_t end   = request.find("\r\n", start);
    std::string lengthStr = request.substr(start, end - start);
    contentLength = std::stoi(lengthStr);
    std::cout << "Content-Length: " << contentLength << std::endl;
  }

  size_t headerEnd  = request.find("\r\n\r\n");
  size_t bodyStart  = (headerEnd != std::string::npos) ? headerEnd + 4 : request.size();
  int    bodyRcvd   = request.size() - bodyStart;
  int    remaining  = contentLength - bodyRcvd;

  while (remaining > 0) {
    int toRead = std::min(remaining, (int)sizeof(buf));
    bytes = client->Read(buf, toRead);
    if (bytes <= 0) break;
    request.append(buf, bytes);
    remaining -= bytes;
  }

  size_t pos = request.find("\r\n\r\n");
  std::string body = (pos != std::string::npos) ? request.substr(pos + 4) : request;

  std::cout << "Body size: " << body.size() << std::endl;

  // Limpiar etiquetas XML para quedarnos con texto plano
  std::regex r("</?[a-zA-Z][a-zA-Z0-9]*[ \".=/]*>");
  std::string result = std::regex_replace(body, r, " ");

  std::string action, figure, contentLine, content;
  std::istringstream stream(result);
  stream >> action >> figure;

  std::cout << "Action: '" << action << "' Figure: '" << figure << "'" << std::endl;

  std::string lineCleaner;
  std::getline(stream, lineCleaner); // descartar resto de la línea
  while (std::getline(stream, contentLine)) {
    content += contentLine + "\n";
  }

  // Quitar basura de inicio/fin
  if (!content.empty() && content.back() == '\n') {
    content.pop_back();
  }
  if (content.size() >= 8) {  // regex suele meter espacios al inicio
    content = content.substr(8);
  }

  std::string fsData;

  if (action == "List") {
    // El Tenedor usa esto para conocer las figuras de este servidor
    fsData = fs->leerUnidad();

  } else if (action == "Request") {
    // Leer figura directamente del FileSystem
    char* temp = fs->leer(figure, MAX_FILESIZE);
    if (temp) {
      fsData = temp;
      delete[] temp;
    } else {
      fsData = "Error: File not found";
    }

  } else if (action == "Submit") {
    fs->crearInodo(figure);
    fs->escribir(figure, content.c_str());
    fsData = fs->leerUnidad();

  } else if (action == "Delete") {
    fs->eliminar(figure);
    fsData = fs->leerUnidad();
  }

  std::string response;
  if (action == "List" || action == "Request" ||
      action == "Submit" || action == "Delete") {

    std::string bodyContent =
        "<html><body><pre>\n" + fsData + "\n</pre></body></html>\n";

    response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(bodyContent.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        bodyContent;
  } else {
    response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><pre>Not Found</pre></body></html>";
  }

  client->Write(response.c_str(), response.size());
}

/**
 * @brief Servicio por conexión (con SSL)
 */
void Service(VSocket* client, FileSystem* fs) {
  try {
    if (auto sslc = dynamic_cast<SSLSocket*>(client)) {
      sslc->Accept();
    }
    processResponse(client, fs);
  } catch (const std::exception& e) {
    std::cerr << "[DRAWING] Error en Service: " << e.what() << std::endl;
  }
  client->Close();
}

/**
 * @brief Lee el contenido de cat.txt para cargar una figura de ejemplo
 */
char* leerFigura(std::streamsize* tArchivo) {
  std::fstream figura("./cat.txt", std::ios::in | std::ios::binary);
  if (!figura.is_open()) {
    std::cerr << "No se pudo abrir la figura" << std::endl;
    return nullptr;
  }

  figura.seekg(0, std::ios::end);
  *tArchivo = figura.tellg();
  figura.seekg(0, std::ios::beg);

  char* buffer = new char[*tArchivo];
  figura.read(buffer, *tArchivo);
  figura.close();
  return buffer;
}

/**
 * @brief main del servidor de figuras (SSL + UDP HELLO)
 */
int main(int cuantos, char** argumentos) {
  FileSystem* fs = new FileSystem();
  fs->crearInodo("gato.dat");

  std::streamsize tArchivo;
  char* buffer = leerFigura(&tArchivo);
  if (buffer) {
    fs->escribir("gato.dat", buffer);
    delete[] buffer;
  }

  VSocket* server;
  VSocket* client;
  std::thread* worker;
  int port = PORTCLIENT;

  if (cuantos > 1) {
    port = std::atoi(argumentos[1]);
  }

#ifdef USE_IPV6
  server = new SSLSocket("Server/ci0123.pem", "Server/key0123.pem", true, true);
#else
  server = new SSLSocket("ci0123.pem", "key0123.pem", true);
#endif

  server->Bind(port);
  server->MarkPassive(10);

  std::cout << "[DRAWING] Servidor (SSL) escuchando en puerto " << port << std::endl;

  //avisamos al tenedor
  notifyTenedor(port);

  for (;;) {
    client = server->AcceptConnection();
    worker = new std::thread(Service, client, fs);
    worker->detach();
  }

  return 0;
}

