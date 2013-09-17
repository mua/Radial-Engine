#include <std.h>
#include "ui_PropertyWidget.h"
#include "rePropertyModel.h"
#include "reCollection.h"
#include <QItemEditorFactory>

class reContextModel;

class rePropertyWidget: public QWidget
{
	Q_OBJECT
public:
	rePropertyWidget(QWidget* parent = 0);

	void setObjects(reObjectList& list);
	reContextModel* contextModel() const;
	void contextModel(reContextModel* val);
	void update();
private:
	rePropertyModel* propertyModel;	
	reContextModel* _contextModel;
	Ui::PropertyWidget ui;
public slots:
	void objectsAboutToBeChanged(reObjectList& objects);
	void objectsChanged();
	void contextStateChanged();
};

class reDefaultItemEditorFactory : public QItemEditorFactory
{
public:
	inline reDefaultItemEditorFactory() {}
	QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
	QByteArray valuePropertyName(QVariant::Type) const;
};

class QBooleanComboBox : public QComboBox
{
	Q_OBJECT
		Q_PROPERTY(bool value READ value WRITE setValue USER true)

public:
	QBooleanComboBox(QWidget *parent);
	void setValue(bool);
	bool value() const;
};
