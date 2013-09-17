#include "reTerrainNode.h"
#include "core.h"
#include "reEditor.h"
#include "QPainter"
#include "reViewport.h"
#include "reTerrainMaterial.h"

#include <QDebug>
#include <QGLContext>
#include <QDir>

reTerrainRenderable::reTerrainRenderable()
{
	rows = cols = 100;
	size = 5;
	shader.vsFileName("C:\\Users\\utku\\Desktop\\cmake\\src\\editor\\assets\\terrain.vp");
	shader.fsFileName("C:\\Users\\utku\\Desktop\\cmake\\src\\editor\\assets\\terrain.fp");
	textureMap.index = 0;
	brushMap.index = 2;
	splatMap.index = 4;
	add(material = new reTerrainMaterial());
}

void reTerrainRenderable::draw( int state /*= 0*/, int id /*= 0*/ )
{
	if (state & reRS_SELECT)
	{
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
	} 
	else
	{
		brushMap.bind();
		textureMap.bind();
		splatMap.bind();
		shader.use();
		shader.setUniform("splatMap", splatMap.index);
		shader.setUniform("brushMap", brushMap.index);
		shader.setUniform("normalMap", textureMap.index + 1);
		material->bind(5, &shader);
	}
	if (!textureMap.mapImage.width())
	{
		return;
	}
	//qDebug() << textureMap.mapImage.format();

	reVec3 offset = reVec3((cols*size*0.5), 0, (rows*size*0.5));	
	float r = 100.0f/255.0f;
	glBegin(GL_TRIANGLES);
	for (int x=0; x<cols; x++)
	{
		for (int y=0; y<rows; y++)
		{			
			reVec3 a = reVec3(x*size, 1, y*size) - offset;
			reVec3 b = reVec3((x+1)*size, 1, (y+1)*size) - offset;
			reVec2 ta(x/(double)cols, y/(double)rows);
			reVec2 tb((x+1)/(double)cols, (y+1)/(double)rows);
			
			reVec2 ma = ta; reVec2 mb = tb;
			mb[1] = 1 - mb[1]; ma[1] = 1 - ma[1];
			ma[0] *= textureMap.mapImage.width()-1; mb[0] *= textureMap.mapImage.width()-1;
			ma[1] *= textureMap.mapImage.height()-1; mb[1] *= textureMap.mapImage.height()-1;
			
			QRgb *c1 = (QRgb *)textureMap.mapImage.scanLine(ma[1]); c1 += (int)ma[0];
			QRgb *c2 = (QRgb *)textureMap.mapImage.scanLine(mb[1]); c2 += (int)ma[0];
			QRgb *c3 = (QRgb *)textureMap.mapImage.scanLine(ma[1]); c3 += (int)mb[0];
			QRgb *c4 = (QRgb *)textureMap.mapImage.scanLine(mb[1]); c4 += (int)mb[0];
			
		
			float h1 = 0; 
			float h2 = 0; 
			float h3 = 0; 
			float h4 = 0; 
			
			h1 = r * qRed(*c1);
			h2 = r * qRed(*c2);
			h3 = r * qRed(*c3);
			h4 = r * qRed(*c4);
			
			//qDebug() << h;
			
			glMultiTexCoord2f(GL_TEXTURE0, ta[0], ta[1]); glVertex3f(a[0], h1, a[2]); 
			glMultiTexCoord2f(GL_TEXTURE0, ta[0], tb[1]); glVertex3f(a[0], h2, b[2]); 
			glMultiTexCoord2f(GL_TEXTURE0, tb[0], tb[1]); glVertex3f(b[0], h4, b[2]); 
			
			glMultiTexCoord2f(GL_TEXTURE0, tb[0], tb[1]); glVertex3f(b[0], h4, b[2]); 
			glMultiTexCoord2f(GL_TEXTURE0, tb[0], ta[1]); glVertex3f(b[0], h3, a[2]); 
			glMultiTexCoord2f(GL_TEXTURE0, ta[0], ta[1]); glVertex3f(a[0], h1, a[2]); 

		}
	}
	glEnd();
		
	glDisable(GL_TEXTURE_2D);
	if (state & reRS_WIREFRAME)
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glColor4f(0,1,0,1);
	}
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);
	shader.unuse();
}

void reTerrainRenderable::load()
{
	textureMap.fileName(mapDirectory() + "/heightmap.png");
}

void reTerrainRenderable::save()
{
	textureMap.textureBuffer->toImage().save((mapDirectory() + "/heightmap.png").c_str());
}

//////////////////////////////////////////////////////////////////////////

reTerrainNode::reTerrainNode()
{
	renderables->add(terrainRenderable = new reTerrainRenderable);
}

std::string reTerrainNode::className()
{
	return "reTerrainNode";
}

reVec2 reTerrainNode::convertPoint( reVec3 point )
{
	reVec4 p = worldTransform().inverseMatrix() * reVec4(point,1);
	float w = terrainRenderable->cols * terrainRenderable->size;
	float h = terrainRenderable->rows * terrainRenderable->size;
	return reVec2(p.x / w + 0.5, p.z / h + 0.5);
}

reVar reTerrainNode::toJson()
{
	reVar ret = reNode::toJson();
	ret["mapDirectory"] = reEditor::instance()->project()->root() + "/.maps/" + name();	
	QDir dir(ret["mapDirectory"].asCString());
	if (!dir.exists())
	{
		dir.mkpath(".");
	}
	terrainRenderable->mapDirectory(ret["mapDirectory"].asString());
	terrainRenderable->save();
	return ret;
}

void reTerrainNode::loadJson( reVar& val )
{
	reNode::loadJson(val);
	if (val.isMember("mapDirectory"))
	{
		terrainRenderable->mapDirectory(val["mapDirectory"].asString());
	}
	terrainRenderable->load();
}
 
//////////////////////////////////////////////////////////////////////////

reEditableMap::reEditableMap()
{	
	unit = 0;
	//mapImage = 0;
	textureSize = reVec2(1024, 1024);
}

void reEditableMap::bind()
{
	if (!unit)
	{
		generate();
	}
	glActiveTexture(GL_TEXTURE0 + index);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, unit);	
}

void reEditableMap::generate()
{
	reEditor::instance()->glWidget->makeCurrent();
	QGLFramebufferObjectFormat  format; 	
	//format.setTextureTarget(GL_TEXTURE_2D);
	format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
	textureBuffer = new QGLFramebufferObject(QSize(textureSize.x, textureSize.y), format);
	format.setSamples(4);
	buffer = new QGLFramebufferObject(QSize(textureSize.x, textureSize.y), format);	
	unit = textureBuffer->texture();
	update();
	reEditor::instance()->currentViewport->makeCurrent();
}

void reEditableMap::update()
{
	//beginEditing();
	draw();
	//endEditing();
}

QPainter* reEditableMap::beginEditing()
{
	reEditor::instance()->glWidget->makeCurrent();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	currentPainter = new QPainter(buffer);
	currentPainter->setRenderHint(QPainter::HighQualityAntialiasing);
	return currentPainter;
}

void reEditableMap::endEditing()
{
	currentPainter->end();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
	QRect rect(0, 0, buffer->width(), buffer->height());
	QGLFramebufferObject::blitFramebuffer(textureBuffer, rect,
		buffer, rect);
	reEditor::instance()->currentViewport->makeCurrent();
}

//////////////////////////////////////////////////////////////////////////

void reIndicatorMap::draw()
{
	
	QPainter* painter = beginEditing();
	painter->fillRect(QRect(0,0,buffer->width(),buffer->height()), Qt::white);
	//painter.drawEllipse(position.x*textureSize.x-50,textureSize.y*(1-position.y)-50,100,100);
	endEditing();
}

//////////////////////////////////////////////////////////////////////////

reTerrainMap::reTerrainMap()
{
	normalFrameBuffer = 0;
	normalShader.vsFileName("C:\\Users\\utku\\Desktop\\cmake\\src\\editor\\assets\\heightnormals.vp");
	normalShader.fsFileName("C:\\Users\\utku\\Desktop\\cmake\\src\\editor\\assets\\heightnormals.fp");
}

void reTerrainMap::draw()
{
	QPainter* painter = beginEditing();
	if (!fileName().empty())
	{
		painter->drawImage(QPoint(0,0), QImage(fileName().c_str()));	
	} 
	else 
	{
		painter->setPen(Qt::blue);
		painter->fillRect(QRect(0,0,buffer->width(),buffer->height()), QColor::fromRgb(100,100,100));
	//	painter->fillRect(QRect(100,100,100,100), Qt::green);
	//	painter->drawPie(QRect(200,200,150,150), 0, 120);
	}
	endEditing();
}

void reTerrainMap::endEditing()
{
	currentPainter->end();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
	QRect rect(0, 0, buffer->width(), buffer->height());
	QGLFramebufferObject::blitFramebuffer(textureBuffer, rect,
		buffer, rect);
	mapImage = buffer->toImage();
	qDebug() << "updated";
	reEditor::instance()->currentViewport->makeCurrent();
	updateNormalMap();
}

void reTerrainMap::generate()
{
	QGLFramebufferObjectFormat format; 
	reEditor::instance()->glWidget->makeCurrent();
	normalTextureBuffer = new QGLFramebufferObject(QSize(textureSize.x, textureSize.y), format);
	format.setSamples(0);
	normalFrameBuffer = new QGLFramebufferObject(textureSize.x, textureSize.y, format);
	normalTexture = normalTextureBuffer->texture();
	reEditor::instance()->currentViewport->makeCurrent();
	reEditableMap::generate();
}

void reTerrainMap::updateNormalMap()
{
	reEditor::instance()->glWidget->makeCurrent();
	normalFrameBuffer->bind();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, unit);	

	normalShader.use();
	normalShader.setUniform("heightmap", 1);

	glViewport(0, 0, 1024, 1024);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1,1,1,1);
	
	//glScalef(0.5, 0.5, 0.5);

	glBegin(GL_QUADS);
	glMultiTexCoord2f(GL_TEXTURE0, 0, 0); glVertex3f(-1,-1, 0); glMultiTexCoord2f(GL_TEXTURE0, 1, 0); glVertex3f( 1,-1, 0);
	glMultiTexCoord2f(GL_TEXTURE0, 1, 1); glVertex3f( 1, 1, 0); glMultiTexCoord2f(GL_TEXTURE0, 0, 1); glVertex3f(-1, 1, 0);
	glEnd();

	normalShader.unuse();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
	normalFrameBuffer->release();
	reEditor::instance()->currentViewport->makeCurrent();
	QRect rect(0, 0, buffer->width(), buffer->height());
	QGLFramebufferObject::blitFramebuffer(normalTextureBuffer, rect,
		normalFrameBuffer, rect);
	//normalFrameBuffer->toImage().save("c:\\normal.png");
}

void reTerrainMap::bind()
{
	reEditableMap::bind();
	glActiveTexture(GL_TEXTURE0 + index + 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, normalTexture);	
}

//////////////////////////////////////////////////////////////////////////

void reSplatMap::draw()
{
	QPainter* painter = beginEditing();
	painter->fillRect(QRect(0,0,buffer->width(),buffer->height()), Qt::white);
	painter->fillRect(QRect(100,100,100,100), Qt::green);
	//painter.drawEllipse(position.x*textureSize.x-50,textureSize.y*(1-position.y)-50,100,100);
	endEditing();
}
