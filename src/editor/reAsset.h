#ifndef reAsset_h__
#define reAsset_h__

#include <QWidget>
#include <QIcon>
#include <QVariant>
#include <QFileInfo>
#include <QString>
#include <json/json.h>

class reAsset;

typedef std::vector<reAsset*> reAssetVector;

class reAsset
{
public:
	QIcon icon;
	QWidget *editor;
	reAssetVector children;
	reAsset* parent;

	reAsset(reAsset* parent);	
	virtual ~reAsset();	

	virtual QWidget *createEditor(QWidget *parent);		

	virtual void load();
	void clearChildren();
	virtual QVariant data(int column) const;
	int columnCount() const;
	reAsset* getChildAt( int row );
	int row();

	void addChild(reAsset* child);
	void removeChild(reAsset* child);

	virtual Json::Value getJson();
	virtual void loadJson(Json::Value& val);

	static reAsset* createChild(const QFileInfo& path, reAsset* parent);
	static reAsset* createChild(QString name, reAsset* parent);	
};
#endif // reAsset_h__