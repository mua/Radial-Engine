#include "std.h"
#include "reMakerAssetLoader.h"
#include "reMaterial.h"
#include <QGLWidget>
#include "rePlayer.h"

// #include "reTerrainNode.h"

void reMakerAssetLoader::loadImage( std::string& fileName, reImage* out)
{
	QImage tex, buf;
	buf.load(filePath(fileName).c_str());
	out->hasAlpha = buf.hasAlphaChannel();
	tex = QGLWidget::convertToGLFormat(buf);
	out->width = tex.width();
	out->height = tex.height();
	out->data = (char*)malloc(tex.byteCount());
	memcpy(out->data, tex.bits(), tex.byteCount());
}

reObject* reMakerAssetLoader::loadObject( reVar& var )
{
//	qDebug() << var.asJson().toStyledString().c_str();
	reObject* node = 0;
	if (var["className"] == "reTerrainNode")
	{
		//node = new reTerrainNode;
		//node->loadJson(json);
	} 
	if (var["className"] == "rePlayer")
	{
		node = new rePlayer;
		node->loadJson(var);
	} 
	return node ? node : reAssetLoader::loadObject(var);
}