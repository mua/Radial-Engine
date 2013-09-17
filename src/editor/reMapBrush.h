#ifndef reMapBrush_h__
#define reMapBrush_h__

#include <QTimer>
#include <QObject>
#include <QColor>

#include "reTypes.h"
#include "reMessage.h"

class QPainter;
class reTerrainNode;
class reEditableMap;
class reTerrainMap;

class reMapBrush: public QObject
{
	Q_OBJECT
private:	
	float _size;
	float _speed;
	float _intensity;
	QTimer* timer;

	QPainter* beginEditing(reVec2 point, reEditableMap* map, bool clean = false);
	void endEditing(reEditableMap* map);
public:
	reVec2 currentPoint;
	reTerrainNode* currentNode;

	virtual reEditableMap* targetMap() = 0;

	bool active;

	reMapBrush();
	virtual void draw(QPainter* painter);
	virtual void drawIndicator(QPainter* painter);

	virtual void mousePress(reTerrainNode* node, reMouseMessage* message, reVec2 point);
	virtual void mouseMove(reTerrainNode* node, reMouseMessage*
		message, reVec2 point);
	virtual void mouseRelease(reTerrainNode* node, reMouseMessage* message, reVec2 point);

	float size() const { return _size; }
	void size(float val) { _size = val; }
	float speed() const;
	void speed(float val);
	float intensity() const { return _intensity; }
	void intensity(float val) { _intensity = val; }
public slots:
	virtual void onTimer();
};

class reHeightBrush: public reMapBrush
{
public:
	enum Direction 
	{
		up,
		down
	};
	Direction direction;
public:
	virtual void draw(QPainter* painter);
	reEditableMap* targetMap();
	//virtual void drawIndicator(QPainter* painter);
};

class reUniformBrush: public reMapBrush
{
private:
	QColor selectedColor;
public:
	void mousePress( reTerrainNode* node, reMouseMessage* message, reVec2 point );
	virtual void draw(QPainter* painter);
	reEditableMap* targetMap();
};

class reSmoothBrush: public reMapBrush
{
public:
	virtual void onTimer();
	reEditableMap* targetMap();
};

class reSplatBrush: public reMapBrush
{
public:
	virtual void draw(QPainter* painter);
	reEditableMap* targetMap();
};

#endif // reMapBrush_h__