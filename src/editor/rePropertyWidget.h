#ifndef rePropertyWidget_h__
#define rePropertyWidget_h__

#include "reNode.h"
#include <QWidget>

class reImporter;
class reAsset;
class reContext;

class rePropertyWidget:
	public QWidget
{
	Q_OBJECT
public:
	reContext* currentContext;
	rePropertyWidget(QWidget* parent);
	void loadProperties(reContext* ctx, reNodeVector& nodes);
	QWidget* editor;
};

#endif // rePropertyWidget_h__
