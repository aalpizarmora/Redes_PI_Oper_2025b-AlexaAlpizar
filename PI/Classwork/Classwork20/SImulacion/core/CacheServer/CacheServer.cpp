#include "CacheServer.hpp"
#include <iostream>

/*// utils
CacheReq Servidor::make_sentinel_() {
  CacheReq s;
  s.ACT = "KILL";
  return s;
}

bool Servidor::is_sentinel_(const CacheReq& m) {
  return m.ACT == "KILL";
}
*/

CacheServer::CacheServer(std::shared_ptr<TSQ<CacheReq>> reqQ,
                         std::shared_ptr<TSQ<CacheReq>> respQ,
                         std::string server_id) {
  this->reqQ_      = std::move(reqQ);
  this->respQ_     = std::move(respQ);
  this->server_id_ = std::move(server_id);
  this->running_   = false;
  this->requests_  = 0;
}

void CacheServer::stop() {
  // TODO: encolar sentinel para salir del loop
  // CacheReq s; s.ACT = "KILL"; reqQ_->enqueue(std::move(s));
}

int CacheServer::run() {
  running_ = true;
  for (;;) {
    CacheReq req = reqQ_->dequeue();
    // if (req.ACT == "KILL") break;
    CacheReq resp = handle(req);

    respQ_->enqueue(std::move(resp));
  }
  running_ = false;
  return 0;
}

CacheReq CacheServer::handle(const CacheReq& req) {
  CacheReq resp = req;

  // acepta "Req" o "REQ"
  const bool isReq  = (resp.ACT == "Req"  || resp.ACT == "REQ");
  const bool isSave = (resp.ACT == "SAVE");

  if (isReq) {
    if (cache_manager_.contains(resp.filename)) {
      resp.estado  = "HIT";
      resp.content = cache_manager_.retrieve(resp.filename);
      std::cout << "[Cache] HIT " << resp.filename << "\n";
      std::cout << "Contenido del cache: " << resp.content << "\n";
      return resp;
    } else {
      resp.estado  = "MISS";
      std::cout << "[Cache] MISS: " << resp.filename << "\n";
    }
    return resp;
  }

  if (isSave) {
    cache_manager_.store(resp.filename, resp.content);
    resp.estado = "SAVED";
    std::cout << "[Cache] Guardado en cache: " << resp.filename << "\n";
    return resp;
  }

  // Acción desconocida
  resp.estado = "ERROR";
  std::cout << "[Cache] ACT desconocido: " << resp.ACT << "\n";
  return resp;
}
