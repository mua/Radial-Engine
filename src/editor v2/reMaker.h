#ifndef reMaker_h__
#define reMaker_h__

#include "std.h"

class reObject;
class reMainWindow;

struct reTypeEntry
{
public:
	QString className;
	QString shortName;
	QStringList accepts;
	QIcon icon;
	virtual reObject* create() const { return 0; };
};

template <class T>
class reType: public reTypeEntry
{
public:
	reType (QString className, QStringList accepts, QString shortName, QIcon icon = QIcon(":/object.png"))
	{
		this->className = className;
		this->accepts = accepts;
		this->shortName = shortName;
		this->icon = icon;
	}
	virtual reObject* create() const
	{
		return new T;
	}
};

class reMaker: public QObject
{
	Q_OBJECT
private:
	QString _root;
public:
	reMainWindow* mainWindow;
	QGLWidget* glWidget;

	reMaker();

	void initGL(QWidget* cnt);
	QString root() const;
	void root(QString val);
	void shutdown();

	void registerType(reTypeEntry* entry);
	const reTypeEntry* getTypeInfo(QString className);
	const reTypeEntry* getTypeInfoByExt( QString ext );
	QList<reTypeEntry*> types;
	QGLContext* ctx;
signals:
	void projectSet();
	void shuttingDown();

public:
	static reMaker* shared();	
};
#endif // reMaker_h__