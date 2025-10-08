#pragma once
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <vector>

#include "../common/tsq.hpp"
#include "../common/Thread.hpp"
#include "../common/Structs.h"   // DealerVerb, Code, DealerMsg, Reply
#include "FileSystem/FileSystem.h"

class Servidor : public Thread {
public:
  Servidor(std::shared_ptr<TSQ<DealerMsg>> reqQ,
           std::shared_ptr<TSQ<Reply>>     respQ,
           FileSystem* fs,
           std::string server_id,
           std::string vlan_id);

  void stop();

protected:
  int run() override;

private:
  Reply handle(const DealerMsg& msg);

  // handlers
  Reply doFetchList_(const DealerMsg& msg);
  Reply doFetchFile_(const DealerMsg& msg);
  Reply doStore_(const DealerMsg& msg);
  Reply doRemove_(const DealerMsg& msg);
  Reply doConnect_(const DealerMsg& msg);
  Reply doKill_(const DealerMsg& msg);

  // utils
  static DealerMsg make_sentinel_();
  static bool      is_sentinel_(const DealerMsg& m);
  static Reply     mk_reply_ok_();
  static Reply     mk_reply_err_(Code c, const std::string& msg);

private:
  std::shared_ptr<TSQ<DealerMsg>> reqQ_;
  std::shared_ptr<TSQ<Reply>>     respQ_;
  FileSystem* fs_{nullptr};

  std::string server_id_;
  std::string vlan_id_;

  std::atomic<bool> running_{false};
  std::atomic<uint64_t> requests_{0};
  std::mutex fs_mtx_;
};