#include "servidor.hpp"
#include <iostream>

// utils
DealerMsg Servidor::make_sentinel_() {
  DealerMsg s;
  s.verb = DealerVerb::KILL;
  return s;
}

bool Servidor::is_sentinel_(const DealerMsg& m) {
  return m.verb == DealerVerb::KILL;
}

Reply Servidor::mk_reply_ok_() {
  Reply r;
  r.code = Code::OK;
  return r;
}

Reply Servidor::mk_reply_err_(Code c, const std::string& msg) {
  Reply r;
  r.code = c;
  r.message = msg;
  return r;
}

Servidor::Servidor(std::shared_ptr<TSQ<DealerMsg>> reqQ,
                   std::shared_ptr<TSQ<Reply>>     respQ,
                   std::shared_ptr<TSQ<CacheReq>> cacheReqQ,
                   std::shared_ptr<TSQ<CacheReq>> cacheRespQ,
                   FileSystem*                     fs,
                   std::string                     server_id,
                   std::string                     vlan_id)
{
  this->reqQ_      = reqQ;
  this->respQ_     = respQ;
  this->cacheReqQ  = cacheReqQ;
  this->cacheRespQ = cacheRespQ;
  this->fs_        = fs;
  this->server_id_ = server_id;
  this->vlan_id_   = vlan_id;
  this->running_   = false;
  this->requests_  = 0;
}


void Servidor::stop() {
  // encola sentinel KILL para salir del loop
  reqQ_->enqueue(make_sentinel_());
}

int Servidor::run() {
  running_ = true;
  for (;;) {
    DealerMsg req = reqQ_->dequeue();
    if (is_sentinel_(req)) break;

    std::cout << "[Servidor " << server_id_ << "] "
              << "verb=" << static_cast<int>(req.verb)
              << " file='" << req.filename << "'\n";

    Reply resp = handle(req);
    respQ_->enqueue(std::move(resp));
  }
  running_ = false;
  return 0;
}

// dispatcher
Reply Servidor::handle(const DealerMsg& msg) {
  requests_.fetch_add(1, std::memory_order_relaxed);

  switch (msg.verb) {
    case DealerVerb::FETCH_LIST: return doFetchList_(msg);
    case DealerVerb::FETCH_FILE: return doFetchFile_(msg);
    case DealerVerb::STORE:      return doStore_(msg);
    case DealerVerb::REMOVE:     return doRemove_(msg);
    case DealerVerb::CONNECT:    return doConnect_(msg);
    case DealerVerb::KILL:       return doKill_(msg);
  }
  return mk_reply_err_(Code::ERROR, "[Servidor] verbo desconocido");
}

// handlers
Reply Servidor::doFetchList_(const DealerMsg&) {
  Reply r = mk_reply_ok_();
  std::lock_guard<std::mutex> lk(fs_mtx_);
  fs_->imprimirDirectorio();
  r.message = "[Servidor] LIST OK ";
  return r;
}

Reply Servidor::doFetchFile_(const DealerMsg& msg) {
  Reply r = mk_reply_ok_();
  if (msg.nbytes <= 0) {
    r.code = Code::ERROR;
    r.message = "[Servidor] nbytes debe ser > 0 para leer " + msg.filename;
    return r;
  }

  // 1) Enviar solicitud al cache
  CacheReq req;
  req.filename = msg.filename;
  req.ACT      = "REQ";
  req.estado   = "";
  req.content  = "";
  cacheReqQ->enqueue(req);
  // 2) Esperar respuesta del cache
  CacheReq resp = cacheRespQ->dequeue();
  // 3) Procesar respuesta
  if (resp.estado == "HIT") {
    std::cout << "[Servidor] CACHE HIT " << msg.filename << std::endl;
    std::cout << "[Servidor] Contenido del cache: " << resp.content << std::endl;
    // Copiar exactamente nbytes, por si hay '\0' en medio
    r.content = resp.content;
    r.message = "[Servidor] CACHE HIT " + msg.filename +
                " (" + std::to_string(msg.nbytes) + " bytes solicitados)";
    return r;
  }else if (resp.estado == "MISS") {
    std::cout << "[Servidor] CACHE MISS " << msg.filename << std::endl;

    // 4) Leer del FileSystem con mutex SOLO alrededor de la lectura
    char* buf = nullptr;
    {
      std::lock_guard<std::mutex> lk(fs_mtx_);
      buf = fs_->leer(msg.filename, msg.nbytes);
    }
    if (!buf) {
      r.code = Code::NOT_FOUND;
      r.message = "[Servidor] " + msg.filename + " no encontrado";
      return r;
    }

    // 5) Guardar en cache (SAVE)
    CacheReq save;
    save.filename = msg.filename;
    save.ACT      = "SAVE";
    save.estado   = "";
    save.content.assign(buf, buf + msg.nbytes);
    cacheReqQ->enqueue(save);
    CacheReq saveResp = cacheRespQ->dequeue();
    // 6) Responder al cliente
    r.content.assign(buf, buf + msg.nbytes);
    delete[] buf;

    r.message = "[Servidor] OK " + msg.filename +
                " (" + std::to_string(msg.nbytes) + " bytes solicitados)";
    return r;
  }else if(resp.estado == "SAVED"){
    std::cout << "[Servidor] CACHE SAVED " << msg.filename << std::endl;
  }else if(resp.estado == ""){
    r.code = Code::ERROR;
    r.message = "[Servidor] Respuesta de caché inválida para " + msg.filename +
              " (estado='" + resp.estado + "')";
  return r;
  }

}

Reply Servidor::doStore_(const DealerMsg& msg) {
  Reply r = mk_reply_ok_();
  std::lock_guard<std::mutex> lk(fs_mtx_);

  const char* data = msg.content.c_str();
  const int bytes  = static_cast<int>(msg.content.size());

  fs_->crearInodo(msg.filename);
  fs_->escribir(msg.filename, msg.content.c_str());
  r.message = "[Servidor] OK stored " + msg.filename +
              " (" + std::to_string(bytes) + " bytes)";
  return r;
}

Reply Servidor::doRemove_(const DealerMsg& msg) {
  Reply r = mk_reply_ok_();

  if (msg.filename.empty()) {
    r.code = Code::ERROR;
    r.message = "[Servidor] DELETE: nombre vacío";
    return r;
  }

  std::lock_guard<std::mutex> lk(fs_mtx_);

  const bool ok = fs_->eliminar(msg.filename);
  if (!ok) {
    r.code = Code::NOT_FOUND;
    r.message = "[Servidor] " + msg.filename + " no existe o no se pudo eliminar";
    return r;
  }

  r.message = "[Servidor] REMOVE OK " + msg.filename;
  return r;
}

Reply Servidor::doConnect_(const DealerMsg&) {
  Reply r = mk_reply_ok_();
  r.message = "[Servidor] CONNECT ACK server=" + server_id_ + " vlan=" + vlan_id_;
  return r;
}

Reply Servidor::doKill_(const DealerMsg&) {
  Reply r = mk_reply_ok_();
  r.message = "[Servidor] KILL ACK; shutting down";
  // apagado ordenado: auto-enfila sentinel para salir del loop
  this->stop();
  return r;
}
