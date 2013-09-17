#include "reRadial.h"

#include <time.h>

#include <iostream>
#include "rePhysics.h"
#include "reInput.h"
#include "reRenderer.h"

#ifdef _WIN32
#include <Windows.h>

double PCFreq = 0.0;
__int64 CounterStart = 0;
bool clockStarted = 0;

float getTime()
{
	//return GetTickCount() / 1000.0f;
	/*
	FILETIME ftime;
	SYSTEMTIME time;
	GetSystemTimeAsFileTime(&ftime);
	FileTimeToSystemTime(&ftime, &time);
	return time.wMilliseconds / 1000.0f + time.wSecond + time.wMinute * 60.0f;
	*/
	if (!clockStarted)
	{
		LARGE_INTEGER li;
		if(!QueryPerformanceFrequency(&li))
		{
			cout << "QueryPerformanceFrequency failed!\n";
			assert(false);
		}
		PCFreq = double(li.QuadPart);
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
		clockStarted = true;
		return 0;
	}
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return float(double(li.QuadPart-CounterStart)/PCFreq);
}
#endif


reRadial* radial = 0;

int count = 0;

reRadial* reRadial::shared()
{
	if (!radial)
	{
		new reRadial;
	}
	return radial;
}

reRadial::reRadial()
{
	radial = this;
	counter = 0;
	input(new reInput);
	//renderer(new reRenderer);
	_renderer = 0;
	//physics(new rePhysics);
}

reAssetLoader* reRadial::assetLoader() const
{
	return _assetLoader;
}

void reRadial::assetLoader( reAssetLoader* val )
{
	_assetLoader = val;
}

int reRadial::registerNode( reNode* node )
{
	//std::cout << "registered: " << count++ << std::endl;
	nodes[++counter] = node;
	return counter;
}

void reRadial::unregisterNode( int handle )
{
	//std::cout << "unregistered: " << count-- << std::endl;
	nodes.erase(handle);
}

reNode* reRadial::getNode( int handle )
{	
	return (nodes.find(handle) != nodes.end()) ? nodes[handle] : 0;
}

float reRadial::time()
{
	return getTime();
}

void reRadial::act()
{
	reMessage message(reM_TIMER);
	dispatchMessage(&message);
}

rePhysics* reRadial::physics() const
{
	return _physics;
}

void reRadial::physics( rePhysics* val )
{
	_physics = val;
}

reInput* reRadial::input() const
{
	return _input;
}

void reRadial::input( reInput* val )
{
	_input = val;
}

reRenderer* reRadial::renderer() const
{
	return _renderer;
}

void reRadial::renderer( reRenderer* val )
{
	_renderer = val;
}
