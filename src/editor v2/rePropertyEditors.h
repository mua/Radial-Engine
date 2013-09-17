#include "reVar.h"

#include "std.h"

#include <string>
#include <vector>

#include "reCollection.h"

class rePropertyEditor;

using namespace std;

typedef vector<rePropertyEditor*> reEditorList;
typedef vector<reVar> reVarList;

class rePropertyEditor
{
private:
	QString _name;
	QString _label;
	rePropertyEditor* _parent;
	bool _editable;	
public:
	reVarList vars;
	reEditorList subEditors;		

	rePropertyEditor();
	virtual ~rePropertyEditor();

	virtual reVar pull(int i);
	virtual void push(reVarList vars);
	virtual void set(reVar val);

	virtual QVariant read(int i = 0);
	virtual void write(QVariant value);

	void add(QString name, rePropertyEditor* editor, QString label = "");

	QString name() const;
	void name(QString val);
	QString label() const;
	void label(QString val);

	rePropertyEditor* parent() const;
	void parent(rePropertyEditor* parent);

	bool editable() const;
	void editable(bool val);

	rePropertyEditor* subEditor( QString name );
	bool isMultiple();
	virtual QVariant display();
};

class reStringEditor: public rePropertyEditor
{
	QVariant read(int i);
	virtual void write(QVariant value);
};

class reNumberEditor: public rePropertyEditor
{
	QVariant read(int i);
	virtual void write(QVariant value);
};

class reIntEditor: public rePropertyEditor
{
	QVariant read(int i);
	virtual void write(QVariant value);
};

class reListEditor: public rePropertyEditor
{
public:
	QMap<int, QString> items;
	QVariant read(int i);
	virtual void write(QVariant value);
	virtual QVariant display();
};

class reVec3Editor: public rePropertyEditor
{
public:
	reVec3Editor();
};

class reTransformEditor: public rePropertyEditor
{
public:
	reTransformEditor();
	void push( reVarList vars );
	reVar pull(int i);
};

class reObjectEditor: public rePropertyEditor
{
public:
	reObjectEditor(reObjectList& objects);
	virtual void push(reVarList vars);
};
