#ifndef rePreviewWidget_h__
#define rePreviewWidget_h__

#include "core.h"
#include <QGLWidget>

class reMesh;
class reNode;

class rePreviewWidget : public QGLWidget
{
	Q_OBJECT

public:
	reMesh* mesh;
	reNode* node;

	rePreviewWidget(QWidget *parent = 0);
	~rePreviewWidget();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void view( reMesh* mesh );
	void view( reNode* node );

	public slots:
		void setXRotation(int angle);
		void setYRotation(int angle);
		void setZRotation(int angle);
		void animate();

signals:
		void xRotationChanged(int angle);
		void yRotationChanged(int angle);
		void zRotationChanged(int angle);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);
private:
	int xRot;
	int yRot;
	int zRot;
	float distance;
	QPoint lastPos;
	QColor qtGreen;
	QColor qtPurple;
};


#endif // rePreviewWidget_h__
