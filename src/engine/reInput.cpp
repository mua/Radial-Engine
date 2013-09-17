#include "reInput.h"
#include "reRadial.h"

reInput::reInput()
{
	memset(keyStates, 0, sizeof(keyStates));
	observe(reRadial::shared(), reM_TIMER);
}

void reInput::keyPress( char key )
{
	//queue.push_back(pair<float, reKeyMessage*>(reRadial::shared()->time(), new reKeyMessage(reM_KEY_PRESS, key)));
	keyStates[key] = true;
	dispatchMessage(&reKeyMessage(reM_KEY_PRESS, key));
}

void reInput::keyUp( char key )
{
	//queue.push_back(pair<float, reKeyMessage*>(reRadial::shared()->time(), new reKeyMessage(reM_KEY_UP, key)));
	keyStates[key] = false;
	dispatchMessage(&reKeyMessage(reM_KEY_UP, key));
}

void reInput::mouseMove( float x, float y )
{
	cursorX = x;
	cursorY = y;
	//dispatchMessage(&reKeyMessage(reM_KEY_UP, key));
}

void reInput::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	vector<pair<float, reKeyMessage*>> newQueue;
	for (int i=0; i<queue.size(); i++)
	{	
		if (reRadial::shared()->time() - queue[i].first >= 0.0)
		{
			dispatchMessage(queue[i].second);
			keyStates[queue[i].second->key] = queue[i].second->id == reM_KEY_PRESS;
			delete queue[i].second;
		} else
			newQueue.push_back(queue[i]);
	}
	queue = newQueue;
}
