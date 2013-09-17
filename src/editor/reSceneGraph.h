#ifndef reSceneGraph_h__
#define reSceneGraph_h__

#include <QTreeView>
#include "core.h"
#include "reNode.h"
#include "reContext.h"

class reSceneModel;

class reSceneGraphWidget:
	public QWidget
{
	Q_OBJECT
private:
	QTreeView* tree;
	reNode* _node;
	reContext *_context;

public:
	reSceneGraphWidget(QWidget* parent = 0);
	virtual ~reSceneGraphWidget();
	reContext * context() const;
	void context(reContext * val);
};

#endif // reSceneGraph_h__