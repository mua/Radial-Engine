#ifndef reInput_h__
#define reInput_h__

#include "reMessage.h"
#include <vector>

class reInput: public reMessageDispatcher, public reMessageObserver
{
public:
	bool keyStates[255];
	float cursorX, cursorY;

	reInput();
	void keyPress(char key);
	void keyUp(char key);
	void mouseMove(float x, float y);

	vector<pair<float, reKeyMessage*>> queue;
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message);
};

#endif // reInput_h__