#include "reEditorAssetLoader.h"
#include "reMaterial.h"

#include <QImage>
#include <gl/glew.h>
#include <QGLWidget>
#include <QDebug>
#include "reContextWindow.h"

#include "reTerrainNode.h"

void reEditorAssetLoader::loadImage( std::string& fileName, reImage* out)
{
	QImage tex, buf;
	buf.load(fileName.c_str());
	out->hasAlpha = buf.hasAlphaChannel();
	tex = QGLWidget::convertToGLFormat(buf);
	out->width = tex.width();
	out->height = tex.height();
	out->data = (char*)malloc(tex.byteCount());
	memcpy(out->data, tex.bits(), tex.byteCount());
}

reObject* reEditorAssetLoader::loadObject( reVar& json )
{
	qDebug() << json.toStyledString().c_str();
	reObject* node = 0;
	if (json["className"] == "reTerrainNode")
	{
		node = new reTerrainNode;
		node->loadJson(json);
	} 
	return node ? node : reAssetLoader::loadObject(json);
}