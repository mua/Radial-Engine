#include "reMapBrush.h"

#include "reTerrainNode.h"

#include <QPainter>
#include <QDebug>

reMapBrush::reMapBrush()
{
	active = false;
	size(0.1f);	
	timer = new QTimer(this);
	speed(0.1f);
	intensity(0.5f);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void reMapBrush::draw( QPainter* painter )
{	
	painter->drawEllipse(QPointF(0.0f, 0.0f), size(), size());
}

void reMapBrush::drawIndicator( QPainter* painter )
{
	static unsigned int offset = 0;
	offset++;
	draw(painter);
	QPen pen(Qt::DashLine);
	pen.setDashOffset(offset/10);
	pen.setColor(Qt::green);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawEllipse(QPointF(0.0f, 0.0f), size(), size());
}

void reMapBrush::mousePress( reTerrainNode* node, reMouseMessage* message, reVec2 point )
{
	active = true;
	timer->start(10/speed());
	currentPoint = point;
	currentNode = node;
	onTimer();
}

void reMapBrush::mouseMove( reTerrainNode* node, reMouseMessage* message, reVec2 point )
{
	QPainter* p = beginEditing(point, &node->terrainRenderable->brushMap, true);
	drawIndicator(p);
	endEditing(&node->terrainRenderable->brushMap);	
	currentPoint = point;
	currentNode = node;
}

void reMapBrush::mouseRelease( reTerrainNode* node, reMouseMessage* message, reVec2 point )
{
	active = false;
	timer->stop();
}

float reMapBrush::speed() const
{
	return _speed;
}

void reMapBrush::speed( float val )
{
	_speed = val;
}

void reMapBrush::onTimer()
{
	QPainter* p = beginEditing(currentPoint, targetMap());
	draw(p);
	endEditing(targetMap());	
}

QPainter* reMapBrush::beginEditing( reVec2 point, reEditableMap* map, bool clean )
{
	QPainter* p;
	reVec2 size;
	size = map->textureSize;
	p = map->beginEditing();
	if (clean)
		p->fillRect(QRect(0,0,size.x, size.y), Qt::white);
	p->setViewTransformEnabled(true);
	QTransform t, s;
	t.translate(point.x, (1-point.y));
	s.scale(size.x, size.y);
	p->setWorldTransform(t*s);
	return p;
}

void reMapBrush::endEditing( reEditableMap* map )
{
	map->endEditing();
}

//////////////////////////////////////////////////////////////////////////

void reHeightBrush::draw( QPainter* painter )
{
	painter->setOpacity(intensity());
	QRadialGradient gradient(0,0,size());
	gradient.setColorAt(0, direction == down ? Qt::black : Qt::red);
	gradient.setColorAt(1, Qt::transparent);
	QBrush brush(gradient);
	painter->setBrush(brush);
	painter->setPen(Qt::NoPen);
	painter->setBackgroundMode(Qt::TransparentMode);
	painter->drawEllipse(QPointF(0.0f, 0.0f), size(), size());
}

reEditableMap* reHeightBrush::targetMap()
{
	return &currentNode->terrainRenderable->textureMap;
}

//////////////////////////////////////////////////////////////////////////

void reUniformBrush::draw( QPainter* painter )
{
	//painter->setOpacity(intensity());
	QRadialGradient gradient(0,0,size());
	gradient.setColorAt(0, selectedColor);
	gradient.setColorAt(1, Qt::transparent);
	QBrush brush(gradient);
	painter->setBrush(brush);
	painter->setPen(Qt::NoPen);
	painter->setBackgroundMode(Qt::TransparentMode);
	painter->drawEllipse(QPointF(0.0f, 0.0f), size(), size());
}

void reUniformBrush::mousePress( reTerrainNode* node, reMouseMessage* message, reVec2 point )
{
	QPoint p(point.x * targetMap()->textureSize.x, (1-point.y) * targetMap()->textureSize.y);
	selectedColor.setRgb(targetMap()->textureBuffer->toImage().pixel(p));
	reMapBrush::mousePress(node, message, point);
}

reEditableMap* reUniformBrush::targetMap()
{
	return &currentNode->terrainRenderable->textureMap;
}

//////////////////////////////////////////////////////////////////////////

QImage blurred(QImage& image, const QRect& rect, int radius, bool alphaOnly = false)
{
	int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

	//QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	QImage &result = image;
	int r1 = rect.top();
	int r2 = rect.bottom();
	int c1 = rect.left();
	int c2 = rect.right();

	int bpl = result.bytesPerLine();
	int rgba[4];
	unsigned char* p;

	int i1 = 0;
	int i2 = 3;

	if (alphaOnly)
		i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

	for (int col = c1; col <= c2; col++) {
		p = result.scanLine(r1) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += bpl;
		for (int j = r1; j < r2; j++, p += bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = result.scanLine(row) + c1 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += 4;
		for (int j = c1; j < c2; j++, p += 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int col = c1; col <= c2; col++) {
		p = result.scanLine(r2) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= bpl;
		for (int j = r1; j < r2; j++, p -= bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = result.scanLine(row) + c2 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= 4;
		for (int j = c1; j < c2; j++, p -= 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	return result;
}

void reSmoothBrush::onTimer()
{
	reEditableMap* map = &currentNode->terrainRenderable->textureMap;
	QPainter* painter = map->beginEditing();
	reVec2 tsize(targetMap()->textureSize.x, targetMap()->textureSize.y);

	reVec2 p(currentPoint.x * tsize.x, (1-currentPoint.y) * tsize.y);
	float w = tsize.x * size() * 2;
	float h = tsize.y * size() * 2;
	QRect r = QRect(p.x-w/2, p.y-h/2, w, h);
	r = r.intersect(QRect(0,0, tsize.x, tsize.y));
	//r = QRect(0,0,targetMap()->textureSize.x, targetMap()->textureSize.y);
	QImage &result = blurred(targetMap()->textureBuffer->toImage(), r, intensity() * 30);
	QBrush brush(result);
	painter->setOpacity(1);
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->setPen(Qt::NoPen);
	painter->setBrush(brush);
	painter->drawEllipse(r);
	map->endEditing();
}

reEditableMap* reSmoothBrush::targetMap()
{
	return &currentNode->terrainRenderable->textureMap;
}

//////////////////////////////////////////////////////////////////////////

void reSplatBrush::draw( QPainter* painter )
{
	painter->setOpacity(intensity());
	QRadialGradient gradient(0,0,size());
	gradient.setColorAt(0, Qt::green);
	gradient.setColorAt(1, Qt::transparent);
	QBrush brush(gradient);
	painter->setBrush(brush);
	painter->setPen(Qt::NoPen);
	painter->setBackgroundMode(Qt::TransparentMode);
	painter->drawEllipse(QPointF(0.0f, 0.0f), size(), size());
}

reEditableMap* reSplatBrush::targetMap()
{
	return &currentNode->terrainRenderable->splatMap;
}
