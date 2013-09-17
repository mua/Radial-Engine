#ifndef reFileAsset_h__
#define reFileAsset_h__

#include <vector>
#include "reAsset.h"

#include <QFileInfo>
#include <QFileIconProvider>

using namespace std;

class reFileAsset:
	public reAsset
{
public:
	QFileInfo info;
	void setPath(QString path);
	reFileAsset( reAsset* _parent );
	virtual ~reFileAsset();

	QString path();
	QString metaPath();
	QString dataDir();

	reFileAsset* nodeByPath(QString path);

	virtual QVariant data( int column ) const;
	virtual void load();
};
#endif // reFileAsset_h__
