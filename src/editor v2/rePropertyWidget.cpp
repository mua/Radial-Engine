#include "std.h"

#include "rePropertyWidget.h"
#include "reContextModel.h"
#include "rePropertyEditors.h"

class rePropertyItemDelegate: public QStyledItemDelegate
{
public:
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		rePropertyEditor* pe = ((rePropertyModel*)index.model())->editor(index);
		if (reListEditor* lpe = dynamic_cast<reListEditor*>(pe))
		{
			QComboBox* editor = new QComboBox(parent);
			editor->addItems(lpe->items.values());
			return editor;
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		rePropertyEditor* pe = ((rePropertyModel*)index.model())->editor(index);
		if (reListEditor* lpe = dynamic_cast<reListEditor*>(pe))
		{
			int value = index.model()->data(index, Qt::EditRole).toInt();
			QComboBox* cb = static_cast<QComboBox*>(editor);
			cb->setCurrentIndex(value);
			return;
		}		
		QStyledItemDelegate::setEditorData(editor, index);
	}

	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		rePropertyEditor* pe = ((rePropertyModel*)index.model())->editor(index);
		if (reListEditor* lpe = dynamic_cast<reListEditor*>(pe))
		{
			QComboBox* cb = static_cast<QComboBox*>(editor);			
			model->setData(index, Qt::EditRole, cb->currentIndex());
			return;
		}
		QStyledItemDelegate::setModelData(editor, model, index);
	}

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const
	{
		QStyledItemDelegate::updateEditorGeometry(editor, option, index);
	}
};

rePropertyWidget::rePropertyWidget( QWidget* parent /*= 0*/ ):
	QWidget(parent)
{
	_contextModel = 0;
	ui.setupUi(this);
	propertyModel = new rePropertyModel();
	ui.tree->setModel(propertyModel);
	connect(propertyModel, SIGNAL(objectsAboutToBeChanged(reObjectList&)), SLOT(objectsAboutToBeChanged(reObjectList&)));
	connect(propertyModel, SIGNAL(objectsChanged()), SLOT(objectsChanged()));

	QItemEditorFactory* factory = new reDefaultItemEditorFactory();
	QItemEditorFactory::setDefaultFactory(factory);
	ui.tree->setAlternatingRowColors(true);
	ui.tree->setItemDelegate(new rePropertyItemDelegate());

}

void rePropertyWidget::update()
{

}

void rePropertyWidget::setObjects(reObjectList& selected)
{
	propertyModel->objects(selected);
	ui.tree->expandAll();
}

reContextModel* rePropertyWidget::contextModel() const
{
	return _contextModel;
}

void rePropertyWidget::contextModel( reContextModel* val )
{
	if (_contextModel)
	{
		disconnect(_contextModel, SLOT(stateChanged()));
	}
	_contextModel = val;
	if (_contextModel)	
	{
		connect(_contextModel, SIGNAL(stateChanged()), SLOT(contextStateChanged()));
	}
}

void rePropertyWidget::objectsAboutToBeChanged( reObjectList& objects )
{
	contextModel()->beginUpdates(objects);
}

void rePropertyWidget::objectsChanged()
{
	contextModel()->endUpdates();
}

void rePropertyWidget::contextStateChanged()
{
	propertyModel->reset();
	ui.tree->expandAll();
}

//////////////////////////////////////////////////////////////////////////

QWidget *reDefaultItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
	switch (type) {
	case QVariant::Bool: {
		QBooleanComboBox *cb = new QBooleanComboBox(parent);
		return cb; }
	case QVariant::UInt: {
		QSpinBox *sb = new QSpinBox(parent);
		sb->setFrame(false);
		sb->setMaximum(INT_MAX);
		return sb; }
	case QVariant::Int: {
		QSpinBox *sb = new QSpinBox(parent);
		sb->setFrame(false);
		sb->setMinimum(INT_MIN);
		sb->setMaximum(INT_MAX);
		return sb; }
	case QVariant::Date: {
		QDateTimeEdit *ed = new QDateEdit(parent);
		ed->setFrame(false);
		return ed; }
	case QVariant::Time: {
		QDateTimeEdit *ed = new QTimeEdit(parent);
		ed->setFrame(false);
		return ed; }
	case QVariant::DateTime: {
		QDateTimeEdit *ed = new QDateTimeEdit(parent);
		ed->setFrame(false);
		return ed; }
	case QVariant::Pixmap:
		return new QLabel(parent);
	case QVariant::Double: {
		QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
		sb->setFrame(false);
		sb->setMinimum(-DBL_MAX);
		sb->setMaximum(DBL_MAX);
		return sb; }
	case QVariant::StringList: {
		QComboBox* cb = new QComboBox(parent);
		return cb;
	}
	case QVariant::String:
	default: {
		// the default editor is a lineedit
		QLineEdit *le = new QLineEdit(parent);
		//le->setFrame(le->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, le));
		//if (!le->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, le))
		//le->setWidgetOwnsGeometry(true);
		return le; }
	}
	return 0;
}

QByteArray reDefaultItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
	switch (type) {
	case QVariant::Bool:
		return "currentIndex";
	case QVariant::UInt:
	case QVariant::Int:
	case QVariant::Double:
		return "value";
	case QVariant::Date:
		return "date";
	case QVariant::Time:
		return "time";
	case QVariant::DateTime:
		return "dateTime";
	case QVariant::StringList:
		return "currentIndex";
	case QVariant::String:
	default:
		return "text";
	}
}

//////////////////////////////////////////////////////////////////////////

QBooleanComboBox::QBooleanComboBox(QWidget *parent)
	: QComboBox(parent)
{
	addItem(QComboBox::tr("False"));
	addItem(QComboBox::tr("True"));
}

void QBooleanComboBox::setValue(bool value)
{
	setCurrentIndex(value ? 1 : 0);
}

bool QBooleanComboBox::value() const
{
	return (currentIndex() == 1);
}