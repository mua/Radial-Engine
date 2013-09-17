#include "reRunner.h"
#include "reRadial.h"
#include "reInput.h"
#include "reScatter.h"
#include "reCamera.h"

reRunner::reRunner()
{
	speed = 3;
	camera = new rePointCamera;
	add(camera, true);
}

void reRunner::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	reInput* input = reRadial::shared()->input();
	switch (message->id)
	{
	case reM_KEY_PRESS:
	case reM_KEY_UP:
		std::cout << "key ";
		direction = reVec3(0, 0, 0);
		if (input->keyStates['W'])
			direction.z = -1;
		if (input->keyStates['S'])
			direction.z = 1;
		if (input->keyStates['A'])
			direction.x = -1;
		if (input->keyStates['D'])
			direction.x = 1;
		break;
	case reM_TIMER:
		float len = glm::length(direction);
		if (len)
		{
			reVec3 velocity = direction*speed / len;
			float dt = reRadial::shared()->time() - time;
			std::cout << dt;
			transform(glm::translate(reMat4(), dt * velocity) * transform().matrix);
		}
		time = reRadial::shared()->time();
		reScatter* scatter = parent()->findObject<reScatter>();
		if (scatter)
		{
			reVec3 pos = reVec3(worldTransform().matrix * reVec4(0,0,0,1));
			reVec3 start(-10, 0, -20), end(10, 0, 1);
			//scatter->bBox(reBBox(pos + end, pos + start));
			scatter->transform(glm::translate(reMat4(), pos));
		}
		reVec3 pos = worldTransform().position();
		camera->lookAt(pos + reVec3(0,1.5,0));
		break;
	}
}

void reRunner::runControl()
{
	observe(reRadial::shared()->input(), reM_KEY_PRESS);
	observe(reRadial::shared()->input(), reM_KEY_UP);
	observe(reRadial::shared(), reM_TIMER);
	time = reRadial::shared()->time();
}

void reRunner::exitControl()
{
	stopObserving(reRadial::shared()->input());
	stopObserving(reRadial::shared());
}

std::string reRunner::className()
{
	return "reRunner";
}