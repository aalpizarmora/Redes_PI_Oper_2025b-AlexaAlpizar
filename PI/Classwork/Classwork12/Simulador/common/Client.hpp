#pragma once
#include <memory>
#include <atomic>
#include <string>
#include <iostream>
#include "../common/tsq.hpp"
#include "../common/Thread.hpp"

class Client : public Thread {
public:
  Client(int clientId,
         std::shared_ptr<TSQ<std::string>> pubReqQ,
         std::shared_ptr<TSQ<std::string>> pubRespQ);
  ~Client();

  int  getId() const;
  void send(const std::string& payload);
  void stop();

protected:
  int run() override;

private:
  int id_{0};
  std::shared_ptr<TSQ<std::string>> pubReqQ_;
  std::shared_ptr<TSQ<std::string>> pubRespQ_;
  std::atomic<bool> running_{false};
};
