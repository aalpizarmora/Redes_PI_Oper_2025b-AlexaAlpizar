#pragma once
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <vector>

#include "../common/tsq.hpp"
#include "../common/Thread.hpp"
#include "../common/Structs.h"     // CacheReq
#include "CacheManager.h"

class CacheServer : public Thread {
public:
  CacheServer(std::shared_ptr<TSQ<CacheReq>> reqQ,
              std::shared_ptr<TSQ<CacheReq>> respQ,
              std::string server_id);

  void stop();

protected:
  int run() override;

private:
  CacheReq handle(const CacheReq& msg);

private:
  CacheManager cache_manager_;
  std::shared_ptr<TSQ<CacheReq>> reqQ_;
  std::shared_ptr<TSQ<CacheReq>> respQ_;
  std::string server_id_;
  std::atomic<bool> running_{false};
  std::atomic<uint64_t> requests_{0};
  std::mutex fs_mtx_;
};
