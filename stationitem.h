#ifndef STATIONITEM_H
#define STATIONITEM_H

#include <QBrush>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>
#include "agent.h"

//
class StationItem : public QGraphicsItem
{
public:
    //StationItem(QColor color, QString stationType, qreal width, qreal height);
    StationItem(QColor color, QString stationType, int stationId, qreal width, qreal height);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString getStateString(Agent::State state);
    void showStationInfo();
    int id;
    QColor color;
    QString type; // Art der Station (fertigungs oder Lade-Station)
    qreal width;
    qreal height;
};

#endif // STATIONITEM_H

/*#ifndef STATIONITEM_H
#define STATIONITEM_H

#include <QBrush>
#include <QGraphicsItem>
#include <QPainter>

class StationItem : public QGraphicsItem
{
public:
    StationItem(QColor color, QString stationType);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QColor color;
    QString type; // Art der Station (normal oder Lade-Station)
};

#endif // STATIONITEM_H
*/
