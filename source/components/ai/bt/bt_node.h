#ifndef _BTNODE_INC
#define _BTNODE_INC

#include <string>
#include <vector>

using namespace std;

class BTController;

class BTControllerNode;

#define RANDOM 0
#define SEQUENCE 1
#define PRIORITY 2
#define ACTION 3
//enum 4 decorator
#define DECORATOR 4

#define STAY 0 
#define LEAVE 1

class BTControllerNode
	{
	string name;
	int type; //es uno de los tipos definidos arriba
	vector<BTControllerNode *>children;
	BTControllerNode *parent;
	BTControllerNode *right;
	public:
		BTControllerNode(string);
		void create(string);
		bool isRoot();
		void setParent(BTControllerNode *);
		void setRight(BTControllerNode *);
		void addChild(BTControllerNode *);
		void setType(int );
		void recalc(BTController *);
		string getName();
};

#endif