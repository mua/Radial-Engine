#ifndef reWidgets_h__
#define reWidgets_h__

#include "std.h"

class reObjectTree: public QTreeView
{
public:
	reObjectTree(QWidget* parent);
	virtual QSize sizeHint() const;
};
#endif // reWidgets_h__
