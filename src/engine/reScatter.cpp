#include "reScatter.h"

reScatter::reScatter()
{
	add(samples = new reSamplesCollection, true);
	samples->name("samples");
	bBox(reBBox(reVec3(-7.5f,0,-7.5f), reVec3(7.5,0,7.5)));
}

std::string reScatter::className()
{
	return "reScatter";
}

void reScatter::parent( reNode* val )
{
	reNode::parent(val);
	if (val)
	{
		observe(val, reM_RESIZE);
		scatter();
	}
}

void reScatter::getTasks( reRenderTask& parent, reTaskList& tasks )
{
	if (samples->count()<2)
	{
		return;
	}
	float cellHeight = 5;
	float cellWidth = 5;

	reVec3 start = reVec3(worldTransform().matrix * reVec4(bBox().pMax, 1));
	reVec3 end	 = reVec3(worldTransform().matrix * reVec4(bBox().pMin, 1));
	reVec3 center = reVec3(worldTransform().matrix * reVec4(0, 0, 0, 1));
	/*
	int endY	= floor(end.z / cellHeight);
	int startY	= floor(start.z / cellHeight);
	int endX	= floor(end.x / cellWidth);
	int startX	= floor(start.x / cellWidth);
	*/

	int endY	= floor(center.z / cellHeight)+1;
	int startY	= floor(center.z / cellHeight)-1;
	int endX	= floor(center.x / cellHeight)+1;
	int startX	= floor(center.x / cellHeight)-1;

	for (int y = startY; y <= endY ; y++)
		for (int x = startX; x <= endX ; x++)
		{
			reRenderTask task(parent);
			int start = tasks.size();
			//task.transform = worldTransform();
			samples->at(abs(y*(endX-startX) + x)%samples->count())->children->at(0)->getTasks(task, tasks);
			reMat4 offsetMatrix = glm::translate(reMat4(), reVec3((x+ 0.5f)*cellWidth, 0, (y+0.5f)*cellHeight));
			for (size_t n=start; n<tasks.size(); n++)
			{
				tasks[n].transform.matrix = offsetMatrix * tasks[n].transform.matrix;
			}
		}
}

void reScatter::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	switch (message->id)
	{
	case reM_RESIZE:
		scatter();
		break;
	}
}

void reScatter::scatter()
{	
	//bBox(reNode::parent() ? reNode::parent()->bBox() : reBBox());
}

bool reScatter::rayTest( reRay& ray )
{
	return false;
}

void reScatter::afterLoad()
{
	reNode::afterLoad();
	for (int i=0; i<samples->count(); i++)
	{
		samples->at(i)->transform(glm::translate(reMat4(), reVec3(i*5,0,0)));
	}
}

reStringList reSamplesCollection::acceptedClasses()
{
	reStringList accepted;
	accepted.push_back("reNode");
	return accepted;
}
