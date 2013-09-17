#include "reFileAsset.h"

#include <typeinfo>

#include <QDir>
#include <QDebug>
#include <QDirIterator>

#include "reFbxAsset.h"
#include "reNodeAsset.h"

reFileAsset::reFileAsset( reAsset* _parent ):
	reAsset(_parent)
{

}

reFileAsset::~reFileAsset()
{

}

QVariant reFileAsset::data( int column ) const
{
	return info.baseName();
}

void reFileAsset::load()
{
	clearChildren(); 
	if (info.isDir()) {
		QDirIterator it(info.filePath(), QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
		while (it.hasNext())
		{			 
			QFileInfo info(it.next());
			if (info.completeSuffix() == "meta") continue;
			reAsset* item = reAsset::createChild(info, this);
			reFileAsset* fileItem = dynamic_cast<reFileAsset*>(item);
			if (fileItem)
			{				
				fileItem->setPath(info.filePath());
			}
			children.push_back(item);
		}
	}
};

void reFileAsset::setPath(QString path)
{
	info = QFileInfo(path);	
	load();
	qDebug() << "loaded:" << path;
}

QString reFileAsset::path()
{
	return info.absoluteFilePath();
};

reFileAsset* reFileAsset::nodeByPath(QString path)
{
	//qDebug() << this->path();
	if (QFileInfo(path).absoluteFilePath() == this->path())
	{
		return this;
	} 
	for (reAssetVector::iterator it=children.begin(); it != children.end(); it++)
	{
		reFileAsset *child = dynamic_cast<reFileAsset*>(*it);
		reFileAsset *item = child ? child->nodeByPath(path): NULL;	
		if (item)
		{
			return item;
		}
	}
	return NULL;
}

QString reFileAsset::metaPath()
{
	QString mp = info.absolutePath() + QDir::separator() + ".meta";
	QDir dir(mp);
	if (!dir.exists(mp))
		QDir().mkdir(mp);
	return mp + QDir::separator() + info.baseName() + ".json";
}

QString reFileAsset::dataDir()
{
	QString mp = info.absolutePath() + QDir::separator() + ".meta" + QDir::separator() + info.baseName();
	QDir dir(mp);
	if (!dir.exists(mp))
		QDir().mkdir(mp);
	return mp;
}

