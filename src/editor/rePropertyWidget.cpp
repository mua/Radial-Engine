#include "rePropertyWidget.h"

#include <QDir>
#include <QFileInfo>
#include <QLayout>
#include <QBoxLayout>
#include "reAsset.h"

#include "reContext.h"

#include "rePropertyEditor.h"

rePropertyWidget::rePropertyWidget( QWidget* parent ):
	QWidget(parent)
{
	editor = NULL;
	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);
}

void rePropertyWidget::loadProperties(reContext* ctx, reNodeVector& nodes)
{
	delete editor; editor = NULL;
	currentContext = ctx;
	if (nodes.size())
	{
		editor = new reNodeEditor(this, currentContext, nodes);
		if (editor)
		{
			layout()->addWidget(editor);
			editor->show();
		}
	}
}
