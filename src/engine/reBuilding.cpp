#include "reBuilding.h"

reBuilding::reBuilding()
{
	add(parts = new rePartsCollection, true);
	add(maps = new rePartsCollection, true);
	add(tiles = new reTilesCollection, true);
	parts->name("parts");
	tiles->name("tiles");
	maps->name("maps");
}

std::string reBuilding::className()
{
	return "reBuilding";
}

void reBuilding::getTasks( reRenderTask& parent, reTaskList& tasks )
{
	reNode::getTasks(parent, tasks);
	return;
	if (!(corner && edge && roof))
	{
		return;
	}
	//corner->getTasks(parent, tasks);
	reMat4 move;

	int x0 = 0, x1 = 5;
	int z0 = 0, z1 = 10;
	for (int x=0; x<=x1; x++)
	{		
		for (int z=0; z<=z1; z++)
		{		
			reRenderTask task(parent);
			move = glm::translate(reMat4(), reVec3(x, 0, z));
			task.transform = move * task.transform.matrix;
			reNode* feature = edge;
			if (x == x0 && z == z0) // left top
			{
				feature = corner;
				task.transform = task.transform.matrix * glm::rotate(reMat4(), -90.0f, reVec3(0,1,0));
			} else if (x == x0 && z == z1) // left bottom
			{
				feature = corner;
				//task.transform = task.transform.matrix * glm::rotate(reMat4(), 90.0f, reVec3(0,1,0));
			}
			else if (x == x1 && z == z0) // right top
			{
				feature = corner;
				task.transform = task.transform.matrix * glm::rotate(reMat4(), 180.0f, reVec3(0,1,0));
			}
			else if (x == x1 && z == z1) // right bottom
			{
				task.transform = task.transform.matrix * glm::rotate(reMat4(), 90.0f, reVec3(0,1,0));
				feature = corner;
			}
			else if (x >= x0 && z == z0) // top edge
			{
				task.transform = task.transform.matrix * glm::rotate(reMat4(), -90.0f, reVec3(0,1,0));
			} else if (x >= x0 && z == z1) // bottom edge
			{
				task.transform = task.transform.matrix * glm::rotate(reMat4(), 90.0f, reVec3(0,1,0));
			}
			else if (x == x1 && z >= z0) // right edge
			{
				//continue;
				task.transform = task.transform.matrix * glm::rotate(reMat4(), 180.0f, reVec3(0,1,0));
			}
			else if (x == x0 && z >= z0) // left edge
			{
				//continue;
			}
			else
			{
				feature = roof;
				//continue;
			}

			feature->getTasks(task, tasks);
			//move *= step;
		}
	}
	//roof->getTasks(parent, tasks);	
	/*
	float cellHeight = 5;
	float cellWidth = 5;

	reVec3 start = reVec3(worldTransform().matrix * reVec4(bBox().pMax, 1));
	reVec3 end	 = reVec3(worldTransform().matrix * reVec4(bBox().pMin, 1));
	reVec3 center = reVec3(worldTransform().matrix * reVec4(0, 0, 0, 1));

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
	*/	
}

void reBuilding::afterLoad()
{
	corner = (reNode*)parts->objectByName("corner", true);
	edge = (reNode*)parts->objectByName("edge", true);
	roof = (reNode*)parts->objectByName("roof", true);

	reVec3 pos = edge->transform().position();
	edge->transform(glm::translate(reMat4(), reVec3(-pos.x, 0, -pos.z)) * edge->transform().matrix);

	pos = corner->transform().position();
	corner->transform(glm::translate(reMat4(), reVec3(-pos.x, 0, -pos.z)) * corner->transform().matrix);

	pos = roof->transform().position();
	roof->transform(glm::translate(reMat4(), reVec3(-pos.x, 0, -pos.z)) * roof->transform().matrix);
	/*
	if (tiles->count())
	{
		reNode* tile1 = (reNode*)tiles->at(0)->children->at(0)->copy();
		reNode* tile2 = (reNode*)tiles->at(0)->children->at(1)->copy();
		reNode* tile3 = (reNode*)tiles->at(0)->children->at(2)->copy();
		reNode* tile4 = (reNode*)tiles->at(0)->children->at(3)->copy();
		reNode* tile5 = (reNode*)tiles->at(0)->children->at(4)->copy();
		reNode* tile6 = (reNode*)tiles->at(0)->children->at(5)->copy();

		children->add(tile1);
		children->add(tile2);
		children->add(tile3);
		children->add(tile4);
		children->add(tile5);
		children->add(tile6);

		tile2->worldTransform(((reNode*)tile1->objectByName("end", true))->worldTransform());
		tile3->worldTransform(((reNode*)tile2->objectByName("end", true))->worldTransform());
		tile4->worldTransform(((reNode*)tile3->objectByName("end", true))->worldTransform());
		tile5->worldTransform(((reNode*)tile4->objectByName("end", true))->worldTransform());
		tile6->worldTransform(((reNode*)tile5->objectByName("end", true))->worldTransform());

	}
	*/
	reNode* map = (reNode*)maps->at(0)->copy();
	map->updateBBox();
	reVec3 move = -map->bBox().center();
	map->transform(glm::translate(reMat4(), move) * map->transform().matrix);
	children->add(map);
	updateBBox();
	for (unsigned int i=0; i<map->children->count(); i++)
	{
		std::string name = map->children->at(i)->name();
		int start = name.find("_", 0)+1;
		int end = name.find("_", start);
		cout << name.substr(start, end - start) << endl;
		reNode* node = (reNode*) tiles->objectByName("tiles_" + name.substr(start, end - start));
		cout << node << endl;
	}
}


//////////////////////////////////////////////////////////////////////////

reStringList rePartsCollection::acceptedClasses()
{
	reStringList accepted;
	accepted.push_back("reNode");
	return accepted;
}

reStringList reTilesCollection::acceptedClasses()
{
	reStringList accepted;
	accepted.push_back("reNode");
	return accepted;
}

