#ifndef _BT_INC
#define _BT_INC

#include <string>
#include <map>
#include "bt_node.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/common/comp_base.h"

using namespace std;

typedef int (BTController::*btaction)();
typedef bool (BTController::*btcondition)();

// Implementation of the behavior tree
// uses the BTnode so both work as a system
// tree implemented as a map of btnodes for easy traversal
// behaviours are a map to member function pointers, to 
// be defined on a derived class. 
// BT is thus designed as a pure abstract class, so no 
// instances or modifications to bt / btnode are needed...


class BTController : public TCompBase {
  // the nodes
  map<string, BTControllerNode *>tree;
  // the C++ functions that implement node actions, hence, the behaviours
  map<string, btaction> actions;
  // the C++ functions that implement conditions
  map<string, btcondition> conditions;

  BTControllerNode *root;
  BTControllerNode *current;

  // moved to private as really the derived classes do not need to see this
  BTControllerNode *createNode(string);
  BTControllerNode *findNode(string);


public:
  string name;
  // use a derived create to declare BT nodes for your specific BTs
  virtual void create(string);
  virtual void updateBT();
  virtual std::string getState();
  // use this two calls to declare the root and the children. 
  // use NULL when you don't want a btcondition or btaction (inner nodes)
  BTControllerNode *createRoot(string, int, btcondition, btaction);
  BTControllerNode *addChild(string, string, int, btcondition, btaction);

  // internals used by btnode and other bt calls
  void addAction(string, btaction);
  int execAction(string);
  void addCondition(string, btcondition);
  bool testCondition(string);
  void setCurrent(BTControllerNode *);


  void onMsgBTPaused(const TMsgBTPaused & msg);
  void debugInMenu();

  // call this once per frame to compute the AI. No need to derive this one, 
  // as the behaviours are derived via btactions and the tree is declared on create
  void update(float delta);
  float dt;

  void setPaused(bool pausedBT);
  bool isPaused();

protected:

  bool _pausedBT = false;

};


#endif