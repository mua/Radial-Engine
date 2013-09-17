#include "reMessage.h"

#include <algorithm>

reMessage::~reMessage()
{
	sender = NULL;	
}

void reMessage::done() 
{
	isDone = true;
}

reMouseMessage::reMouseMessage( reMessageId id, reVec3& p, reVec3& dir, int buttons ):
	reMessage()
{
	this->id = id;
	this->p = p;
	this->dir = dir;
	this->buttons = buttons;
	isDone = false;
}

reKeyMessage::reKeyMessage( reMessageId id, char key ):
	reMessage(id)
{
	this->key = key;
}


//////////////////////////////////////////////////////////////////////////

void reMessageDispatcher::addObserver( reMessageObserver* observer, reMessageId id )
{
	if (isObserving(id, observer)) return;
	observers[id].push_back(observer);
}

void reMessageDispatcher::insertObserver( reMessageObserver* observer, reMessageId id, int index )
{
	if (isObserving(id, observer)) return;
	if (index > -1)
	{
		observers[id].insert(observers[id].begin() + index, observer);
	}
	else
		addObserver(observer, id);
}

void reMessageDispatcher::removeObserver( reMessageObserver* observer, reMessageId id )
{
	reMessageObserverVector &vec = observers[id];
	reMessageObserverVector::iterator it = std::find(vec.begin(), vec.end(), observer);
	if (it != vec.end())
	{
		vec.erase(it);
	}
}

void reMessageDispatcher::removeObserver( reMessageObserver* observer )
{
	for (reMessageObserverMap::iterator mit = observers.begin(); mit!=observers.end(); mit ++)
	{
		reMessageObserverVector::iterator it = find(mit->second.begin(), mit->second.end(), observer);
		if (it != mit->second.end())
		{
			mit->second.erase(it);
		}
	}
}

void reMessageDispatcher::dispatchMessage( reMessage* message )
{
	dispatching = true;
	reMessageObserverVector vec = observers[message->id];
	for (reMessageObserverVector::iterator it = vec.begin(); it != vec.end(); it++)
	{
		if (!message->isDone)
		{
			(*it)->messageProcess(this, message);
		}		
	}
	dispatching = false;
}

reMessageDispatcher::~reMessageDispatcher()
{
	if (dispatching)
	{
		__debugbreak();
	}
	for (reMessageObserverMap::iterator mit = observers.begin(); mit!=observers.end(); mit ++)
	{
		for (reMessageObserverVector::iterator it = mit->second.begin(); it != mit->second.end(); it++)
		{
			(*it)->stopObserving(this, false);
		}
	}
}

bool reMessageDispatcher::isObserving( reMessageId id, reMessageObserver* observer )
{
	reMessageObserverVector &vec = observers[id];
	reMessageObserverVector::iterator it = std::find(vec.begin(), vec.end(), observer);
	return it != vec.end();
}

reMessageDispatcher::reMessageDispatcher()
{
	dispatching = false;
}
//////////////////////////////////////////////////////////////////////////

void reMessageObserver::observe( reMessageDispatcher* dispatcher, reMessageId id, int index )
{
	if (find(dispatchers.begin(), dispatchers.end(), dispatcher) == dispatchers.end())
		dispatchers.push_back(dispatcher);
	dispatcher->insertObserver(this, id, index);
}

reMessageObserver::~reMessageObserver()
{
	for (reMessageDispatcherVector::iterator it = dispatchers.begin(); it != dispatchers.end(); it++) 
	{
		(*it)->removeObserver(this);
	}
}

void reMessageObserver::stopObserving( reMessageDispatcher* dispatcher, bool notifyDispatcher )
{
	if (notifyDispatcher)
		dispatcher->removeObserver(this);
	reMessageDispatcherVector &vec = dispatchers;
	reMessageDispatcherVector::iterator it = std::find(vec.begin(), vec.end(), dispatcher);
	if (it != vec.end())
	{
		vec.erase(it);
	}
}

