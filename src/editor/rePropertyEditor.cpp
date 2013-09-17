#include "rePropertyEditor.h"
#include "reContext.h"

#include <QMouseEvent>
#include <QScrollArea>
#include <QLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QItemEditorFactory>
#include <QComboBox>
#include <QTextedit>
#include <QStandardItemEditorCreator>
#include <QDebug>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLabel>
#include <QLineEdit>

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

class reDefaultItemEditorFactory : public QItemEditorFactory
{
public:
	inline reDefaultItemEditorFactory() {}
	QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
	QByteArray valuePropertyName(QVariant::Type) const;
};

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
	case QVariant::String:
	default:
		return "text";
	}
}

reNodeEditor::reNodeEditor( QWidget* parent, reContext* ctx, reNodeVector& nodes ):
	QWidget(parent), properties()
{
	this->nodes = nodes;
	/*
	manager = new QtVariantPropertyManager(this);	
	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);

	browser = new QtTreePropertyBrowser;
	browser->setFactoryForManager(manager, variantFactory);	

	QScrollArea* scroll = new QScrollArea;	
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	*/
	properties = new reNodeProperties();
	properties->setNodes(nodes);

	/*scroll->setWidget(browser);
	scroll->setWidgetResizable(true);

	QBoxLayout* boxLayout = new QVBoxLayout;
	boxLayout->addWidget(scroll);
	*/
	
	QBoxLayout* boxLayout = new QVBoxLayout;
	QItemEditorFactory* factory = new reDefaultItemEditorFactory();
	//factory->registerEditor(QVariant::Bool, new QStandardItemEditorCreator<QCheckBox>);
	//factory->registerEditor(QVariant::String, new QStandardItemEditorCreator<QTextEdit>);
	QItemEditorFactory::setDefaultFactory(factory);

	//qDebug() << QItemEditorFactory::defaultFactory();
	model = new rePropertiesModel();
	model->ctx = ctx;
	model->root = &properties->all;
	rePropertyTree* tree = new rePropertyTree(this);
	tree->setModel(model);
	tree->expandAll();
	tree->setEditTriggers(QAbstractItemView::CurrentChanged);
	tree->setAlternatingRowColors(true);

	boxLayout->setContentsMargins(0,0,0,0);
	boxLayout->setSpacing(0);

	boxLayout->addWidget(tree);

	setLayout(boxLayout);

	connect(ctx, SIGNAL(nodesUpdated(reNodeVector&)), SLOT(nodesUpdated(reNodeVector&)));
	setContentsMargins(0,0,0,0);
	
}

void reNodeEditor::nodesUpdated( reNodeVector& nodes )
{
	model->reload();
}

//////////////////////////////////////////////////////////////////////////

QModelIndex rePropertiesModel::index( rePropertySet* pset ) const
{
	if (pset->parent)
	{
		int pos = std::find(pset->parent->subsets.begin(), pset->parent->subsets.end(), pset) - pset->parent->subsets.begin();
		return index(pos, 0, index(pset->parent));
	}
	return QModelIndex();
}

QModelIndex rePropertiesModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	rePropertySet* ps = parent.isValid() ? set(parent) : root;
	return createIndex(row, column, ps->subsets[row]);
}

QModelIndex rePropertiesModel::parent( const QModelIndex &child ) const
{
	return child.isValid() ? index(set(child)->parent) : QModelIndex();
}

int rePropertiesModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (parent.column()>0) return 0;
	if (!parent.isValid())
	{
		return root->subsets.size();
	}
	rePropertySet* pn = set(parent);
	return pn ? pn->subsets.size() : root->subsets.size();
}

int rePropertiesModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 2;
}

QVariant rePropertiesModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	if (!index.isValid()) return QVariant();
	switch (role)
	{
	case Qt::DisplayRole:
		return !index.column() ? set(index)->name : set(index)->get();
	case Qt::EditRole:
		return index.column() ? set(index)->get() : QVariant();
	default:
		return QVariant();
	}
}

bool rePropertiesModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/ )
{
	if (!index.column() == 1) return false;
	switch (role)
	{
	case Qt::EditRole:
		ctx->beginUpdatingObjects(ctx->selectedObjects());
		set(index)->set(value);
		ctx->endUpdatingObjects();
	}
	return false;
}

rePropertySet* rePropertiesModel::set( const QModelIndex& index ) const
{
	return index.isValid() ? (rePropertySet*)index.internalPointer(): 0;
}

Qt::ItemFlags rePropertiesModel::flags( const QModelIndex &index ) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | (index.isValid() && index.column() && set(index)->editable() ? Qt::ItemIsEditable : 0);
}

void rePropertiesModel::reload()
{
	emit dataChanged(QModelIndex(), QModelIndex());
}

QVariant rePropertiesModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role == Qt::DisplayRole)
	{
		return section ? "Value" : "Property";
	}
	return QVariant();
}
//////////////////////////////////////////////////////////////////////////

reNodeProperties::reNodeProperties()
{
	transform.parent = &all;
	all.subsets.push_back(&transform);
	worldTransform.parent = &all;
	all.subsets.push_back(&worldTransform);
}

void reNodeProperties::createProperties( reNode* node )
{
	typedef reTransformProperty<reNode, &reNode::transform, &reNode::transform> reNodeTransformProperty;	
	reNodeTransformProperty* nodeTransform = new reNodeTransformProperty;
	nodeTransform->setHost(node);
	transform.addProperty(nodeTransform);

	typedef reTransformProperty<reNode, &reNode::worldTransform, &reNode::worldTransform> reWorldTransformProperty;
	reWorldTransformProperty* wTransform = new reWorldTransformProperty;
	wTransform->setHost(node);
	worldTransform.addProperty(wTransform);
}

void reNodeProperties::setNodes( reNodeVector& nodes )
{
	for (reNodeVector::iterator it=nodes.begin(); it != nodes.end(); it++)
	{
		createProperties(*it);
	}
}

//////////////////////////////////////////////////////////////////////////

rePropertyTree::rePropertyTree( QWidget* parent ):
QTreeView(parent)
{

}

void rePropertyTree::mousePressEvent( QMouseEvent *event )
{
	QTreeView::mousePressEvent(event);
	QModelIndex idx = indexAt(event->pos());
	qDebug() << idx.row() << idx.column();
	if ((model()->flags(idx) & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))
	{
		setCurrentIndex(idx);
		edit(idx);
	}
	/*
	if (idx.isValid()) {
		if ((item != m_editorPrivate->editedItem()) && (event->button() == Qt::LeftButton)
			&& (header()->logicalIndexAt(event->pos().x()) == 1)
			&& ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))) {
				editItem(item, 1);
		} else if (!m_editorPrivate->hasValue(item) && m_editorPrivate->markPropertiesWithoutValue() && !rootIsDecorated()) {
			if (event->pos().x() + header()->offset() < 20)
				item->setExpanded(!item->isExpanded());
		}
	}
	*/
}

