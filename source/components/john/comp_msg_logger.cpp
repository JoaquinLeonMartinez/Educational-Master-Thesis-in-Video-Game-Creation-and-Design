#include "mcv_platform.h"
#include "engine.h"

class TCompMsgLogger : public TCompBase {

  std::vector< std::string > msgs;

  void log(const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    char dest[1024];
    _vsnprintf(dest, sizeof(dest), format, argptr);
    va_end(argptr);
    
    msgs.push_back(std::string(dest));
    dbg("%s", dest );
  }

  void onTriggerEnter(const TMsgEntityTriggerEnter& trigger_enter) {
    CEntity* e = trigger_enter.h_entity;
    log("onTriggerEnter %s\n", e->getName());
  }

  void onTriggerExit(const TMsgEntityTriggerExit& trigger_exit) {
    CEntity* e = trigger_exit.h_entity;
    log("onTriggerExit %s\n", e->getName());
  }

public:
  void debugInMenu() {
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear"))
      msgs.clear();
    for( size_t i=0; i<msgs.size(); ++i)
      ImGui::Text(msgs[msgs.size()-1-i].c_str());
  }
  static void registerMsgs() {
    DECL_MSG(TCompMsgLogger, TMsgEntityTriggerEnter, onTriggerEnter);
    DECL_MSG(TCompMsgLogger, TMsgEntityTriggerExit, onTriggerExit);
  }
};

DECL_OBJ_MANAGER("msg_logger", TCompMsgLogger);

