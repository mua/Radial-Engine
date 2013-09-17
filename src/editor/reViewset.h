#ifndef reViewset_h__
#define reViewset_h__

#include <QMainWindow>
#include <QSplitter>

#include "reNode.h"

class reViewport;

typedef std::vector<reViewport*> reViewportList;
class reViewset:
	public QWidget
{
	Q_OBJECT
public:
	reViewportList viewports;
	reViewset(QWidget* parent = 0);

	QWidget * wrap( QWidget * wrapped );
	reNode* node() const;
	void node(reNode* val);
	reNode* helperNode() const;
	void helperNode(reNode* val);
private:
	QSplitter *vs1, *vs2, *hs;	

	bool centerDragging;
	reNode *_node, *_helperNode;
	void resizeEvent(QResizeEvent *e);
private slots:
	void closing();	
	void splitterMoved1 ( int pos, int index );
	void splitterMoved2 ( int pos, int index );
	bool eventFilter(QObject *obj, QEvent *event);
	void nodeClickedVP(reNode*);
	void nodesSelectedVP(reNodeVector& nodes);
	

signals:
	void nodeClicked(reNode*);
	void nodesSelected(reNodeVector& nodes);
};

#endif // reViewset_h__