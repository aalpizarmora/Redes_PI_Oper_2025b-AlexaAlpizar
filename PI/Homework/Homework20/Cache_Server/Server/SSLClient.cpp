// Server/SSLClient.cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "SSLSocket.h"   // incluye VSocket/Socket internamente

static std::string buildXml(const std::string& action,
                            const std::string& figure,
                            const std::string& content)
{
    std::ostringstream xml;
    xml << "<Body>\n"
        << "  <Action>"  << action  << "</Action>\n"
        << "  <Figure>"  << figure  << "</Figure>\n"
        << "  <Content>" << content << "</Content>\n"
        << "</Body>";
    return xml.str();
}

static std::string buildHttpPost(const std::string& host,
                                 const std::string& xmlBody)
{
    std::ostringstream req;
    req << "POST / HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Type: application/xml\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << xmlBody.size() << "\r\n"
        << "\r\n"
        << xmlBody;
    return req.str();
}

// Lee exactamente n bytes del socket (bloqueante) hasta completar o fallar
static bool readExactly(VSocket* sock, std::string& out, size_t n)
{
    out.clear();
    out.reserve(n);
    char buf[1024];
    while (out.size() < n) {
        const size_t want = std::min(n - out.size(), sizeof(buf));
        int bytes = sock->Read(buf, want);
        if (bytes <= 0) return false;
        out.append(buf, bytes);
    }
    return true;
}

// Lee una respuesta HTTP y devuelve el cuerpo (si trae <pre>, extrae el contenido de <pre>…</pre>)
static std::string readHttpResponse(VSocket* sock)
{
    // 1) Lee headers hasta \r\n\r\n
    std::string headers;
    headers.reserve(1024);
    {
        char c;
        std::string window;
        window.reserve(4);
        while (true) {
            int r = sock->Read(&c, 1);
            if (r <= 0) break;
            headers.push_back(c);

            window.push_back(c);
            if (window.size() > 4) window.erase(window.begin());
            if (window.size() == 4 && window == "\r\n\r\n") break;
        }
    }

    // 2) Parsear Content-Length (opcional pero recomendado)
    size_t hdrEnd = headers.find("\r\n\r\n");
    std::string headerOnly = (hdrEnd == std::string::npos) ? headers : headers.substr(0, hdrEnd);

    size_t contentLen = 0;
    {
        // Buscar "Content-Length:"
        std::string hl = headerOnly;
        // normaliza un poco a minúsculas para buscar robusto (opcional)
        for (char& ch : hl) ch = (char)tolower((unsigned char)ch);

        const std::string key = "content-length:";
        size_t p = hl.find(key);
        if (p != std::string::npos) {
            // tomar la línea a partir de p + key.size()
            size_t lineEnd = headerOnly.find("\r\n", p);
            std::string value = headerOnly.substr(p + key.size(), lineEnd - (p + key.size()));
            // recortar espacios
            size_t a = value.find_first_not_of(" \t");
            size_t b = value.find_last_not_of(" \t");
            if (a != std::string::npos) value = value.substr(a, b - a + 1);
            contentLen = (size_t)std::stoul(value);
        }
    }

    // 3) Lee el cuerpo exactamente Content-Length (si existe); si no existe, lee hasta EOF
    std::string body;
    if (contentLen > 0) {
        if (!readExactly(sock, body, contentLen)) {
            // fallback: intentar leer lo que haya hasta EOF
            char buf[1024];
            int bytes;
            while ((bytes = sock->Read(buf, sizeof(buf))) > 0) {
                body.append(buf, bytes);
            }
        }
    } else {
        // No hubo CL: leer hasta EOF
        char buf[1024];
        int bytes;
        while ((bytes = sock->Read(buf, sizeof(buf))) > 0) {
            body.append(buf, bytes);
        }
    }

    // 4) Si hay <pre>…</pre>, extraerlo
    size_t preStart = body.find("<pre>");
    size_t preEnd   = body.find("</pre>");
    if (preStart != std::string::npos && preEnd != std::string::npos && preEnd > preStart + 5) {
        return body.substr(preStart + 5, preEnd - (preStart + 5));
    }
    return body;
}

static bool doOneRequest(const std::string& host, int port,
                         const std::string& action,
                         const std::string& figure,
                         const std::string& content)
{
    try {
        // Cliente SSL en modo cliente (server=false, ipv6? depende de tu constructor)
        SSLSocket client(/*cert*/false, /*isServer*/false);
        client.MakeConnection(host.c_str(), port); // hace SSL_connect adentro en tu wrapper

        // arma request
        const std::string xml = buildXml(action, figure, content);
        const std::string httpReq = buildHttpPost(host, xml);

        // envía
        client.Write(httpReq.c_str(), httpReq.size());

        // recibe
        std::string resp = readHttpResponse(&client);
        std::cout << "----- RESPONSE BODY (sanitized) -----\n"
                  << resp << "\n------------------------------------\n";
        client.Close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[client] error: " << e.what() << "\n";
        return false;
    }
}

int main(int argc, char* argv[])
{
    const std::string host = (argc > 1) ? argv[1] : "localhost";
    const int port         = (argc > 2) ? std::atoi(argv[2]) : 6752;

    std::cout << "SSL Client to https://" << host << ":" << port << "\n";
    std::cout << "Acciones: List, Request, Submit, Delete, Quit\n";

    while (true) {
        std::string action, figure, content;

        std::cout << "\nAccion> ";
        if (!std::getline(std::cin, action)) break;
        if (action.empty()) continue;

        // normaliza capitalize primera letra por comodidad
        if (!action.empty()) {
            action[0] = (char)toupper((unsigned char)action[0]);
            for (size_t i = 1; i < action.size(); ++i)
                action[i] = (char)tolower((unsigned char)action[i]);
        }

        if (action == "Quit" || action == "Q" || action == "Exit") {
            std::cout << "Saliendo...\n";
            break;
        }

        if (action == "List") {
            figure.clear();
            content.clear();
        } else if (action == "Request") {
            std::cout << "Figura> ";
            std::getline(std::cin, figure);
            content.clear();
        } else if (action == "Submit") {
            std::cout << "Figura> ";
            std::getline(std::cin, figure);
            std::cout << "Contenido (linea unica; si necesitas multilinea, pega \\n manualmente)>\n";
            std::getline(std::cin, content);
        } else if (action == "Delete") {
            std::cout << "Figura> ";
            std::getline(std::cin, figure);
            content.clear();
        } else {
            std::cout << "Accion invalida.\n";
            continue;
        }

        (void)doOneRequest(host, port, action, figure, content);
    }

    return 0;
}
