#include "Client.hpp"
#include <iostream>

Client::Client(
  int clientId,
  std::shared_ptr<TSQ<std::string>> pubReqQ,
  std::shared_ptr<TSQ<std::string>> pubRespQ)
{
  this->id_ = clientId;
  this->pubReqQ_  = pubReqQ;
  this->pubRespQ_ = pubRespQ;
}

Client::~Client() {}

int Client::getId() const { return this->id_; }

void Client::send(const std::string& payload) {
  if (!this->pubReqQ_) {
    std::cerr << "[Cliente " << this->id_ << "] ERROR: cola publica no inicializada\n";
    return;
  }
  this->pubReqQ_->enqueue(payload);
  std::cout << "[Cliente " << this->id_ << "] -> encola: " << payload << "\n";
}

void Client::stop() {
  this->running_.store(false, std::memory_order_relaxed);
}

int Client::run() {
  this->running_.store(true, std::memory_order_relaxed);
  if (!this->pubRespQ_) {
    std::cerr << "[Cliente " << this->id_ << "] ERROR: cola publica resp no inicializada\n";
    return 1;
  }
  while (this->running_.load(std::memory_order_relaxed)) {
    const std::string respuesta = this->pubRespQ_->dequeue();
    if (!this->running_.load(std::memory_order_relaxed)) break;
    std::cout << "[Cliente " << this->id_ << "] He recibido " << respuesta << '\n';
  }
  return 0;
}
