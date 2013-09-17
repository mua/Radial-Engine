#ifndef reTerrainModifier_h__
#define reTerrainModifier_h__

#include "reModifier.h"

class reTerrainPanel;
class reTerrainNode;
class reMapBrush;

class reTerrainModifier: 
	public reModifier, public reMessageObserver
{
private:
	reTerrainPanel* panel;	
	reMapBrush* _brush;
public:
	reTerrainNode* terrain;
	reTerrainModifier(QObject* parent);
	virtual void attach(reContextWindow* ctxWindow);
	virtual void detach();

	void messageProcess(reMessageDispatcher* sender, reMessage* message);
	reMapBrush* brush() const { return _brush; }
	void brush(reMapBrush* val) { _brush = val; }
};

#endif // reTerrainModifier_h__