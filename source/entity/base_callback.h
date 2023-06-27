#pragma once

struct IMsgBaseCallback {
  // La interfaz virtual quiere un handle y una direccion al msg
  virtual void sendMsg(CHandle h_recv, const void* msg) = 0;
};

struct TCallbackSlot {
  uint32_t          comp_type;
  IMsgBaseCallback* callback;
};

extern std::unordered_multimap< uint32_t, TCallbackSlot > all_registered_msgs;

