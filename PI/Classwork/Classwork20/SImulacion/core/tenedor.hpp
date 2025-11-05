#pragma once
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "../common/tsq.hpp"
#include "../common/Thread.hpp"
#include "../common/Structs.h"

class Tenedor : public Thread {
public:
  Tenedor(std::shared_ptr<TSQ<std::string>> pubReqQ,
          std::shared_ptr<TSQ<std::string>> pubRespQ,
          std::shared_ptr<TSQ<DealerMsg>>   privReqQ,
          std::shared_ptr<TSQ<Reply>>       privRespQ);
  ~Tenedor();

  void stop();

  protected:
  int run() override;

private:
  void response_loop_();
  DealerMsg toDealerMsg_(const std::string& text);
  std::string replyToText_(const Reply& rep);

private:
  std::shared_ptr<TSQ<std::string>> pubReqQ_;
  std::shared_ptr<TSQ<std::string>> pubRespQ_;
  std::shared_ptr<TSQ<DealerMsg>>   privReqQ_;
  std::shared_ptr<TSQ<Reply>>       privRespQ_;
  std::thread resp_thread_;
  std::atomic<bool> running_{false};
};
