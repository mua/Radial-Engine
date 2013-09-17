#ifndef reMessage_h__
#define reMessage_h__

#include "reTypes.h"
#include "reTransform.h"

#include <vector>
#include <map>


typedef enum {
	reM_NONE = 0,
	reM_MOUSE_PRESS = 1,
	reM_MOUSE_RELEASE = 2,
	reM_MOUSE_MOVE = 3,
	reM_MOUSE_ENTER = 4,
	reM_MOUSE_LEAVE = 5,
	
	reM_NODE_DESTRUCT = 6,
	reM_NODE_ADD_RENDERABLE = 8,
	reM_TRANSFORM = 9,

	reM_KEY_PRESS = 10,
	reM_KEY_UP = 11,

	reM_TIMER = 7,

	reM_RESIZE = 8
} reMessageId;

class reNode;

class reMessage
{	
public:
	reMessageId id;
	reNode* sender;
	bool isDone;

	reMessage (reMessageId id = reM_NONE)  {this->id = id; isDone = false;};
	virtual ~reMessage();

	void done();
};

class reMouseMessage:
	public reMessage
{
public:
	reVec3 p, dir;
	int buttons;
	reMouseMessage(reMessageId id, reVec3& p, reVec3& dir, int buttons);
};

class reKeyMessage: public reMessage
{
public:
	char key;
	reKeyMessage(reMessageId id, char key);
};

class reRenderable;

class reMessageRenderable:
	public reMessage
{
public:
	reMessageRenderable(): reMessage() {};
	reRenderable* renderable;
	reTransform transform;
};

class reMessageObserver;
class reMessageDispatcher;

typedef std::vector<reMessageObserver*> reMessageObserverVector;
typedef std::vector<reMessageDispatcher*> reMessageDispatcherVector;
typedef std::map<reMessageId, reMessageObserverVector> reMessageObserverMap;

class reMessageObserver
{
public:
	reMessageDispatcherVector dispatchers;
	void observe(reMessageDispatcher* dispatcher, reMessageId id, int index = -1);
	void stopObserving(reMessageDispatcher* dispatcher, bool notifyDispatcher = true);
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) = 0;
	virtual ~reMessageObserver();
};

class reMessageDispatcher
{
public:
	bool dispatching;
	reMessageObserverMap observers;
	reMessageDispatcher();
	void addObserver(reMessageObserver* observer, reMessageId id);
	void insertObserver( reMessageObserver* observer, reMessageId id, int index );

	bool isObserving( reMessageId id, reMessageObserver* observer );
	void removeObserver(reMessageObserver* observer, reMessageId id);
	void removeObserver( reMessageObserver* observer );
	void dispatchMessage(reMessage* message);
	virtual ~reMessageDispatcher();	
};

#endif // reMessage_h__