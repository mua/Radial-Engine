#include "std.h"
#include "reWidgets.h"

reObjectTree::reObjectTree( QWidget* parent ) :
QTreeView(parent)
{

}

QSize reObjectTree::sizeHint() const
{
	return QSize(500, height());
}

