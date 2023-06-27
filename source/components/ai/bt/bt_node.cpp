#include "mcv_platform.h"
#include <stdio.h>

#include "bt_node.h"
#include "bt_controller.h"


BTControllerNode::BTControllerNode(string s)
{
  name = s;
}


void BTControllerNode::create(string s)
{
  name = s;
}


bool BTControllerNode::isRoot()
{
  return (parent == NULL);
}


void BTControllerNode::setParent(BTControllerNode *p)
{
  parent = p;
}


void BTControllerNode::setRight(BTControllerNode *p)
{
  right = p;
}


void BTControllerNode::setType(int t)
{
  type = t;
}


string BTControllerNode::getName()
{
  return name;
}


void BTControllerNode::addChild(BTControllerNode *c)
{
  if (!children.empty()) // if this node already had children, connect the last one to this
    children.back()->setRight(c);  // new one so the new one is to the RIGHT of the last one
  children.push_back(c); // in any case, insert it
  c->right = NULL; // as we're adding from the right make sure right points to NULL
}


void BTControllerNode::recalc(BTController *tree)
{
  if (!tree->isPaused()) {
    // activate the next line to debug
    printf("recalcing node %s\n", name.c_str());
    switch (type)
    {
    case ACTION:
    {
      // run the controller of this node
      int res = tree->execAction(name);
      // now, the next lines compute what's the NEXT node to use in the next frame...
      if (res == STAY) { tree->setCurrent(this); return; }// looping vs. on-shot actions
                                                          // climb tree iteratively, look for the next unfinished sequence to complete
      BTControllerNode *cand = this;
      while (cand->parent != NULL)
      {
        BTControllerNode *daddy = cand->parent;
        if (daddy->type == SEQUENCE)
          // oh we were doing a sequence. make sure we finished it!!!
        {
          if (cand->right != NULL)
          {
            tree->setCurrent(cand->right);
            break;
          }
          // sequence was finished (there is nobody on right). Go up to daddy.
          else cand = daddy;
        }
        // i'm not on a sequence, so keep moving up to the root of the BT
        else cand = daddy;
      }
      // if we've reached the root, means we can reset the traversal for next frame.
      if (cand->parent == NULL) tree->setCurrent(NULL);
      break;
    }
    case RANDOM:
    {//supone que tienes hijos 
		int res = tree->execAction(name);
      int r = rand() % children.size();
      children[r]->recalc(tree);
      break;
    }
    case PRIORITY:
    {
		int res = tree->execAction(name);
      for (int i = 0; i < children.size(); i++)
      {
        if (tree->testCondition(children[i]->getName()))
        {
          children[i]->recalc(tree);
          break;
        }
      }
      break;
    }
    case SEQUENCE:
    {
      // begin the sequence...the inner node (action) will take care of the sequence
      // via the "setCurrent" mechanism
		int res = tree->execAction(name);
      children[0]->recalc(tree);
      break;
    }
    }
  }
}
