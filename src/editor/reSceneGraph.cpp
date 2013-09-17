#include "reSceneGraph.h"
#include "reContext.h"
#include "reRefNode.h"
#include "reRadial.h"
#include "reEditorAssetLoader.h"
#include <QSettings>
#include <QBoxLayout>
#include <QTreeView>
#include <QTGui>

reSceneGraphWidget::reSceneGraphWidget( QWidget* parent /*= 0*/ ):
	QWidget(parent)
{	
	tree = new QTreeView(this);
	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);
	l->addWidget(tree);
}

reSceneGraphWidget::~reSceneGraphWidget()
{

}

reContext * reSceneGraphWidget::context() const
{
	return _context;
}

void reSceneGraphWidget::context( reContext * val )
{
	_context = val;
	tree->setModel(NULL);
	tree->setModel(context()->model);
	tree->setSelectionModel(context()->selectionModel);

	//tree->setDragDropMode(QAbstractItemView::InternalMove);

	tree->setDragEnabled(true);	
	tree->setAcceptDrops(true);
	tree->setDropIndicatorShown(true);

	tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	//tree->setExpandsOnDoubleClick(false);
	tree->header()->hide();
}