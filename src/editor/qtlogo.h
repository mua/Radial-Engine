#ifndef QTLOGO_H
#define QTLOGO_H

#include <QObject>
#include <QColor>

class Patch;
struct Geometry;

//! [0]
class QtLogo : public QObject
{
public:
    QtLogo(QObject *parent, int d = 64, qreal s = 1.0);
    ~QtLogo();
    void setColor(QColor c);
    void draw() const;
private:
    void buildGeometry(int d, qreal s);

    QList<Patch *> parts;
    Geometry *geom;
};
//! [0]

#endif // QTLOGO_H
