#include "tenedor.hpp"
#include <iostream>

Tenedor::Tenedor(std::shared_ptr<TSQ<std::string>> pubReqQ,
                 std::shared_ptr<TSQ<std::string>> pubRespQ,
                 std::shared_ptr<TSQ<DealerMsg>>   privReqQ,
                 std::shared_ptr<TSQ<Reply>>       privRespQ)
  : pubReqQ_(std::move(pubReqQ))
  , pubRespQ_(std::move(pubRespQ))
  , privReqQ_(std::move(privReqQ))
  , privRespQ_(std::move(privRespQ)) {}

Tenedor::~Tenedor() {
  if (resp_thread_.joinable()) resp_thread_.join();
}

void Tenedor::stop() {
  running_.store(false, std::memory_order_relaxed);
  DealerMsg kill; kill.verb = DealerVerb::KILL;   // TODO: implementar cierre en servidor
  privReqQ_->enqueue(kill);
}

int Tenedor::run() {
  running_.store(true, std::memory_order_relaxed);
  resp_thread_ = std::thread([this]{ this->response_loop_(); });

  while (running_.load(std::memory_order_relaxed)) {
    const std::string text = pubReqQ_->dequeue();
    if (!running_.load(std::memory_order_relaxed)) break;
    if (text.empty()) {
      std::cout << "[TENEDOR] comando vacio\n";
      continue;
    }
    DealerMsg dmsg = toDealerMsg_(text);
    std::cout << "[TENEDOR] enviado verbo " << static_cast<int>(dmsg.verb)
              << " archivo '" << dmsg.filename << "'\n";
    privReqQ_->enqueue(std::move(dmsg));
  }

  if (resp_thread_.joinable()) resp_thread_.join();
  return 0;
}

void Tenedor::response_loop_() {
  while (running_.load(std::memory_order_relaxed)) {
    Reply rep = privRespQ_->dequeue();
    if (!running_.load(std::memory_order_relaxed)) break;
    pubRespQ_->enqueue(replyToText_(rep));
  }
}

DealerMsg Tenedor::toDealerMsg_(const std::string& text) {
  DealerMsg m;

  // separar comando y argumento
  const auto pos = text.find(' ');
  const std::string func = (pos == std::string::npos) ? text : text.substr(0, pos);
  const std::string arg  = (pos == std::string::npos) ? ""   : text.substr(pos + 1);

  if (func == "LIST") {
    m.verb = DealerVerb::FETCH_LIST;
    return m;
  }

  if (func == "REQUEST" || func == "GET") {
    // Formato esperado: REQUEST <nombre>|<nbytes>
    m.verb = DealerVerb::FETCH_FILE;

    const auto bar = arg.rfind('|');
    if (bar == std::string::npos) {
      throw std::runtime_error("REQUEST requiere formato: REQUEST <nombre>|<nbytes>");
    }

    m.filename = arg.substr(0, bar);
    const std::string nbytes_str = arg.substr(bar + 1);

    if (m.filename.empty()) {
      throw std::runtime_error("REQUEST: nombre vacío");
    }
    try {
      m.nbytes = std::stoi(nbytes_str);
    } catch (...) {
      throw std::runtime_error("REQUEST: nbytes inválido");
    }
    if (m.nbytes <= 0) {
      throw std::runtime_error("REQUEST: nbytes debe ser > 0");
    }
    return m;
  }

  if (func == "SUBMIT" || func == "ADD" || func == "STORE") {
    // Formato: SUBMIT <nombre>|<contenido>
    m.verb = DealerVerb::STORE;
    const auto bar = arg.find('|');

    if (bar == std::string::npos) {
      // Permitimos SUBMIT <nombre> (sin contenido) para compatibilidad
      m.filename = arg;
      m.content.clear();
      m.nbytes = 0;
      if (m.filename.empty()) {
        throw std::runtime_error("SUBMIT: nombre vacío");
      }
      return m;
    }

    m.filename = arg.substr(0, bar);
    m.content  = arg.substr(bar + 1);
    if (m.filename.empty()) {
      throw std::runtime_error("SUBMIT: nombre vacío");
    }
    m.nbytes = static_cast<int>(m.content.size());  // útil si el servidor quiere saber tamaño
    return m;
  }

  if (func == "DELETE" || func == "DEL" || func == "REMOVE") {
    m.verb = DealerVerb::REMOVE;
    m.filename = arg;
    if (m.filename.empty()) {
      throw std::runtime_error("DELETE: nombre vacío");
    }
    return m;
  }

  if (func == "CONNECT" || func == "PING") {
    m.verb = DealerVerb::CONNECT;
    return m;
  }

  if (func == "KILL") {
    m.verb = DealerVerb::KILL;
    return m;
  }

  // Default simple (puedes decidir lanzar error en lugar de CONNECT)
  throw std::runtime_error(std::string("Comando desconocido: ") + func);
}

std::string Tenedor::replyToText_(const Reply& rep) {
  std::string out;
  if (!rep.message.empty()) out += rep.message;
  if (!rep.content.empty()) {
    if (!out.empty()) out += "\n";
    out += rep.content;
  }
  if (!rep.list.empty()) {
    if (!out.empty()) out += "\n";
    for (size_t i = 0; i < rep.list.size(); ++i) {
      out += rep.list[i];
      if (i + 1 < rep.list.size()) out += ", ";
    }
  }
  return out;
}
