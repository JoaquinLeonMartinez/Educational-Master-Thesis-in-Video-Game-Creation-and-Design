#include "mcv_platform.h"
#include "bt_controller.h"


void BTController::create(string s)
{
  name = s;
}


BTControllerNode *BTController::createNode(string s)
{
  if (findNode(s) != NULL)
  {
    printf("Error: node %s already exists\n", s.c_str());
    return NULL;	// error: node already exists
  }
  BTControllerNode *BTControllern = new BTControllerNode(s);
  tree[s] = BTControllern;
  return BTControllern;
}


BTControllerNode *BTController::findNode(string s)
{
  if (tree.find(s) == tree.end()) return NULL;
  else return tree[s];
}


BTControllerNode *BTController::createRoot(string s, int type, btcondition BTControllerc, btaction BTControllera)
{//comprobar que no este ya creado son find node
  BTControllerNode *r = createNode(s);
  r->setParent(NULL);
  root = r;

  //esto es igual siempre
  r->setType(type);
  if (BTControllerc != NULL) addCondition(s, BTControllerc);
  if (BTControllera != NULL) addAction(s, BTControllera);

  current = NULL;
  return r;
}


BTControllerNode *BTController::addChild(string parent, string son, int type, btcondition BTControllerc, btaction BTControllera) //padre, hijo , tipo, condicion, accion
{
  BTControllerNode *p = findNode(parent);
  BTControllerNode *s = createNode(son);
  p->addChild(s);
  s->setParent(p);

  //esto es igual siempre
  s->setType(type);
  if (BTControllerc != NULL) addCondition(son, BTControllerc);
  if (BTControllera != NULL) addAction(son, BTControllera);
  return s;
}

void BTController::updateBT() {

}

std::string BTController::getState() {
	return "undefined";
}

void BTController::update(float delta)
{
  if (!_pausedBT) {
	  updateBT();
    if (current == NULL) root->recalc(this);	// I'm not in a sequence, start from the root
    else current->recalc(this);				// I'm in a sequence. Continue where I left
	
  }
  dt = delta;
}

void BTController::setPaused(bool pausedBT)
{
  _pausedBT = pausedBT;
}

bool BTController::isPaused()
{
  return _pausedBT;
}

void BTController::onMsgBTPaused(const TMsgBTPaused & msg)
{
  _pausedBT = msg.isPaused;
}


void BTController::setCurrent(BTControllerNode *nc)
{
  current = nc;
}


void BTController::addAction(string s, btaction act)
{
  if (actions.find(s) != actions.end())
  {
    printf("Error: node %s already has an action\n", s.c_str());
    return;	// if action already exists don't insert again...
  }
  actions[s] = act;
}


int BTController::execAction(string s)
{
  if (actions.find(s) == actions.end())
  {
    printf("ERROR: Missing node action for node %s\n", s.c_str());
    return LEAVE; // error: action does not exist
  }
  return (this->*actions[s])();
}


void BTController::addCondition(string s, btcondition cond)
{
  if (conditions.find(s) != conditions.end())
  {
    printf("Error: node %s already has a condition\n", s.c_str());
    return;	// if condition already exists don't insert again...
  }
  conditions[s] = cond;
}


bool BTController::testCondition(string s)
{
  if (conditions.find(s) == conditions.end())
  {
    return true;	// error: no condition defined, we assume TRUE
  }
  return (this->*conditions[s])();;
}

void BTController::debugInMenu() {
	
}

