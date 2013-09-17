#ifndef reNode_h__
#define reNode_h__

#include <core.h>
#include <string>
#include <vector>
#include <string>
#include "reTransform.h"
#include "reTypes.h"
#include "reMessage.h"
#include "reCollection.h"
#include "reRenderable.h"
#include "reRenderer.h"
#include <json/json.h>

using namespace std;

class reMessage;
class reNodeCollection;

typedef vector<reNode*> reNodeVector;
typedef vector<reRenderable*> reRenderableVector;

typedef enum {
	reWS_GO = 0,
	reWS_PASS = 1,
	reWS_COMPLETE = 2,
	reWS_ADD = 4
} reWalkState;

class reRenderableCollection: public reTypedCollection<reRenderable>
{
public:
	virtual string className();
	virtual void toJson(reVar& val, int categories = ALL_PROPERTIES) override;
	virtual void loadJson(reVar& value);
	virtual void add(reObject* object, bool fixed = 0);
	virtual reStringList acceptedClasses();
};

class reNode: 
	public reCollection, public reMessageDispatcher, public reRenderClient
{
private:
	reNode* _parent;
	reBBox _bBox;

	reTransform _transform;
	reTransform _worldTransform;

public:
	int id;
	reNodeCollection* children;
	reRenderableCollection* renderables;

public:
	reNode();
	virtual ~reNode();

	virtual void toJson(reVar& val, int categories = ALL_PROPERTIES) override;
	virtual void loadJson(reVar& val);
	virtual void save(string& fileName);
	virtual void load(string& fileName);

	virtual string className();

	reTransform transform() const;
	void transform(reTransform val);
	reTransform worldTransform() const;
	void worldTransform(reTransform val);
	reTransform objectTransform() const;
	void updateWorldTransform();

	reNode* parent() const;
	virtual void parent(reNode* val);
	reBBox bBox() const;
	void bBox(const reBBox val);
	void updateBBox();

	virtual void render(int state = 0);

	virtual void processMessage(reMessage* message);

	reWalkState walk(reWalkState (*callback)(reNode*, void*), void* ctx=0);
	void filter(reWalkState (*callback)(reNode*, void*), void* ctx, reNodeVector& out);	
	bool isChildNode(reNode* child);
	virtual reStringList acceptedClasses() override;
	virtual void afterLoad();

	virtual void updateShader(reShader* shader) override;
	virtual void getTasks(reRenderTask& parent, reTaskList& tasks);
};

class reNodeCollection: public reTypedCollection<reNode>
{
public:
	virtual void insert(reObject* object, int index);
	virtual string className();
	virtual reStringList acceptedClasses();
};

#endif // reNode_h__
