#pragma once
#include <string>
#include <vector>

// verbos del canal privado Tenedor <-> Servidor
enum class DealerVerb { FETCH_LIST, FETCH_FILE, STORE, REMOVE, CONNECT, KILL };

// codigos de respuesta del servidor
enum class Code { OK, NOT_FOUND, UNAVAILABLE, ERROR };

// mensaje privado hacia el servidor
struct DealerMsg {
  DealerVerb verb {DealerVerb::CONNECT};
  std::string filename;   // para FETCH_FILE / STORE / REMOVE
  std::string content;    // para STORE
  int nbytes {0};         // bytes de content
};

// respuesta privada desde el servidor
struct Reply {
  Code code {Code::OK};
  std::string message;              // texto
  std::string content;              // contenido de archivo (GET)
  std::vector<std::string> list;    // lista de archivos (LIST)
};

// estructura para el cache
struct CacheReq {
  std::string filename;
  std::string content;
  std::string estado; //MISS, HIT
  std::string ACT; //Req o SAVE
};