#include "std.h"
#include "rePropertyEditors.h"

#include "reTypes.h"
#include "reTransform.h"
#include "reVar.h"

#include "reNode.h"
#include "reRefNode.h"
#include "reAnimationSet.h"
#include "reBody.h"

void rePropertyEditor::push( reVarList vars )
{
	this->vars = vars;	
	for (reEditorList::iterator it = subEditors.begin(); it!=subEditors.end(); it++)
	{
		reVarList list;
		for (reVarList::iterator vit = vars.begin(); vit != vars.end(); vit++)
		{
			reVar &var = *vit;
			if (var.isMember((*it)->name().toStdString()))
			{
				list.push_back(var[(*it)->name().toStdString()]);
			}
		}
		(*it)->push(list);
	}
}

reVar rePropertyEditor::pull(int i) 
{
	reVar var = vars[i];
	for (reEditorList::iterator it = subEditors.begin(); it!=subEditors.end(); it++)
	{
		if (var.isMember((*it)->name().toStdString()))
		{
			var[(*it)->name().toStdString()] = (*it)->pull(i);
		}
	}
	return var;
}

void rePropertyEditor::set( reVar val )
{
	reVarList list;
	for (unsigned int i =0; i<vars.size(); i++)
	{
		list.push_back(val);
	}
	push(list);
}

QVariant rePropertyEditor::read(int i)
{
	return QVariant();
}

void rePropertyEditor::write( QVariant value )
{

}

QString rePropertyEditor::name() const
{
	return _name;
}

void rePropertyEditor::name( QString val )
{
	_name = val;
}

rePropertyEditor* rePropertyEditor::parent() const
{
	return _parent;
}

void rePropertyEditor::parent( rePropertyEditor* val )
{
	_parent = val;
}

void rePropertyEditor::add(QString name, rePropertyEditor* editor, QString label )
{
	editor->name(name);
	editor->label(label.isEmpty() ? name : label);
	editor->parent(this);
	subEditors.push_back(editor);
}

QString rePropertyEditor::label() const
{
	return _label;
}

void rePropertyEditor::label( QString val )
{
	_label = val;
}

bool rePropertyEditor::editable() const
{
	return _editable;
}

void rePropertyEditor::editable( bool val )
{
	_editable = val;
}

rePropertyEditor::rePropertyEditor()
{
	editable(true);
	parent(0);
}

rePropertyEditor::~rePropertyEditor()
{
	for (reEditorList::iterator it=subEditors.begin(); it!=subEditors.end(); it++)
	{
		delete *it;
	}
}

rePropertyEditor* rePropertyEditor::subEditor( QString name )
{
	for (reEditorList::iterator it = subEditors.begin(); it!=subEditors.end(); it++)
	{
		if ((*it)->name() == name)
		{
			return *it;
		}
	}
	return 0;
}

bool rePropertyEditor::isMultiple()
{
	for (unsigned int i =0; i<vars.size(); i++)
	{
		if (read(i) != read(0))
		{
			return true;
		}
	}
	return false;
}

QVariant rePropertyEditor::display()
{
	return read();
}

//////////////////////////////////////////////////////////////////////////

reTransformEditor::reTransformEditor()
{
	add("position", new reVec3Editor);
	add("rotation", new reVec3Editor);
	add("scale", new reVec3Editor);
}

void reTransformEditor::push( reVarList _vars )
{
	reTransform transform;
	reVarList vars;
	for (unsigned int i =0; i<_vars.size(); i++)
	{
		reVar &val = _vars[i];
		for (unsigned int i=0; i<val.size(); i++)
		{
			transform.matrix[i/4][i%4] = val[i].asFloat();
		}
		vars.push_back(transform);
	}
	rePropertyEditor::push(vars);
}

reVar reTransformEditor::pull( int i )
{
	reVar val = rePropertyEditor::pull(i);
	reTransform transform = reTransform(val);

	reVar transformValue;
	for (unsigned int i =0; i<16; i++)
	{
		transformValue[i] = transform.matrix[i/4][i%4];
	}
	return transformValue;
}

reVec3Editor::reVec3Editor()
{
	add("x", new reNumberEditor);
	add("y", new reNumberEditor);
	add("z", new reNumberEditor);
}

template<typename T>
bool allInherited(reObjectList& list)
{
	for (reObjectList::iterator it = list.begin(); it != list.end(); it++)
	{
		if (!dynamic_cast<T*>(*it))
		{
			return false;
		}
	}
	return true;
}

reObjectEditor::reObjectEditor(reObjectList& objects)
{
	add("name", new reStringEditor);
	add("path", new reStringEditor, "Path");

	if (allInherited<reNode>(objects))
	{
		add("transform", new reTransformEditor);
	}
	if (allInherited<reSequence>(objects))
	{
		add("start", new reNumberEditor);
		add("end", new reNumberEditor);
	}
	if (allInherited<reBody>(objects))
	{
		reListEditor* le = new reListEditor;
		le->items[0] = "Static";
		le->items[1] = "Kinetic";
		le->items[2] = "Dynamic";
		add("kind", le);
		add("mass", new reNumberEditor);
	}

	/*
	if (allInherited<reRefNode>(objects))
	{		
	}
	*/
}

void reObjectEditor::push( reVarList vars )
{
	rePropertyEditor::push(vars);
	subEditors[0]->editable(!subEditors[0]->isMultiple());
}
//////////////////////////////////////////////////////////////////////////

QVariant reStringEditor::read(int i)
{
	return vars[i].asString().c_str();
}

void reStringEditor::write( QVariant val )
{
	set(val.toString().toStdString());
}

//////////////////////////////////////////////////////////////////////////

QVariant reNumberEditor::read(int i)
{
	double f = vars[i].asFloat();
	if (abs(f)<0.0001) f = 0;
	return QVariant(f);
}

void reNumberEditor::write( QVariant val )
{
	set(val.toFloat());
}

//////////////////////////////////////////////////////////////////////////


QVariant reIntEditor::read( int i )
{
	return QVariant(vars[i].asInt());
}

void reIntEditor::write( QVariant value )
{
	set(value.toInt());
}

//////////////////////////////////////////////////////////////////////////

QVariant reListEditor::read( int i )
{	
	return items.keys().indexOf(vars[i].asInt());
}

void reListEditor::write( QVariant value )
{	
	int idx = value.toInt();
	int key = items.keys()[idx];
	set(key);
}

QVariant reListEditor::display() 
{
	return items.values()[read(0).toInt()];
}
