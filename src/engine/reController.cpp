#include "reController.h"

reController::reController():
started(false)
{

}

void reController::start()
{
	if (!started)
	{
		started = true;
		initControl();
	}
	runControl();
}

void reController::pause()
{
	pauseControl();
}

void reController::stop()
{
	if (!started)
		return;
	started = false;
	exitControl();
	initControl();
}

void reController::shutdown()
{
	started = false;
	exitControl();
}

void reController::initControl()
{

}

void reController::runControl()
{

}

void reController::pauseControl()
{

}

void reController::exitControl()
{

}
