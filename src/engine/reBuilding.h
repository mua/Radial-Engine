#include "reNode.h"

class rePartsCollection: public reTypedCollection<reNode>
{
	virtual reStringList acceptedClasses() override;
};

class reTilesCollection: public reTypedCollection<reNode>
{
	virtual reStringList acceptedClasses() override;
};

class reBuilding: public reNode

{
public:
	reNode *corner, *edge, *roof;
	rePartsCollection *parts;
	rePartsCollection *maps;
	reTilesCollection* tiles;
	reBuilding();
	virtual std::string className() override;
	virtual void getTasks( reRenderTask& parent, reTaskList& tasks ) override;
	virtual void afterLoad();
};

