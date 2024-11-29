#ifndef STATION_H
#define STATION_H

#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>

//
class Station {
public:
    Station(const QString& nameLeft, const QString& nameRight, int x, int y, QGraphicsScene* scene);

    // Methoden zum Setzen der Farben für innere Rechtecke und Werkbänke
    void setStateByName(const QString& name, const QColor& color);
    void setWorkbenchStateByName(const QString& name, int workbenchNumber, const QColor& color);

    // Getter für die Namen der inneren Rechtecke
    QString getLeftName() const { return leftName; }
    QString getRightName() const { return rightName; }

private:
    QString leftName;
    QString rightName;

    QGraphicsRectItem* leftRect;
    QGraphicsRectItem* rightRect;

    QGraphicsEllipseItem* leftWorkbench1;
    QGraphicsEllipseItem* leftWorkbench2;
    QGraphicsEllipseItem* rightWorkbench1;
    QGraphicsEllipseItem* rightWorkbench2;
};

#endif // STATION_H
