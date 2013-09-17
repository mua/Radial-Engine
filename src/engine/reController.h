#ifndef reController_h__
#define reController_h__

class reController
{
public:
	bool started;

	reController();

	virtual void start();
	virtual void pause();
	virtual void stop();
	virtual void shutdown();

	virtual void initControl();
	virtual void pauseControl();
	virtual void exitControl();
	virtual void runControl();
};

#endif // reController_h__