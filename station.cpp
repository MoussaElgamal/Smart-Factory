#include "station.h"

/*Station::Station(const QString& nameLeft, const QString& nameRight, int x, int y, QGraphicsScene* scene)
    : leftName(nameLeft), rightName(nameRight)
{
    int stationWidth = 200;
    int stationHeight = 100;
    int innerRectWidth = stationWidth / 2;
    int workbenchRadius = 20;

    // Äußeres Rechteck (Container, ohne Sichtbarkeit)
    QGraphicsRectItem *outerRect = new QGraphicsRectItem(0, 0, stationWidth, stationHeight);
    outerRect->setPos(x, y);
    outerRect->setBrush(Qt::NoBrush);
    scene->addItem(outerRect);

    // Linkes inneres Rechteck
    leftRect = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
    leftRect->setPos(0, 0);
    leftRect->setBrush(Qt::lightGray);  // Standardfarbe
    scene->addItem(leftRect);

    // Fett und schwarz setzen
    QFont boldFont("Arial", 12, QFont::Bold);

    // Beschriftung für linkes inneres Rechteck
    QGraphicsTextItem* labelLeft = new QGraphicsTextItem(nameLeft, leftRect);
    labelLeft->setPos(10, 10);
    scene->addItem(labelLeft);

    // Rechtes inneres Rechteck
    rightRect = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
    rightRect->setPos(innerRectWidth, 0);
    rightRect->setBrush(Qt::lightGray);  // Standardfarbe
    scene->addItem(rightRect);




    // Beschriftung für rechtes inneres Rechteck
    QGraphicsTextItem* labelRight = new QGraphicsTextItem(nameRight, rightRect);
    labelRight->setPos(10, 10);
    scene->addItem(labelRight);

    // Werkbänke für linkes inneres Rechteck
    leftWorkbench1 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, leftRect);
    leftWorkbench1->setPos(30, 20);
    scene->addItem(leftWorkbench1);

    leftWorkbench2 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, leftRect);
    leftWorkbench2->setPos(30, 60);
    scene->addItem(leftWorkbench2);

    // Werkbänke für rechtes inneres Rechteck
    rightWorkbench1 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, rightRect);
    rightWorkbench1->setPos(30, 20);
    scene->addItem(rightWorkbench1);

    rightWorkbench2 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, rightRect);
    rightWorkbench2->setPos(30, 60);
    scene->addItem(rightWorkbench2);
}
*/
#include "station.h"

Station::Station(const QString& nameLeft, const QString& nameRight, int x, int y, QGraphicsScene* scene)
    : leftName(nameLeft), rightName(nameRight)
{
    int stationWidth = 200;
    int stationHeight = 100;
    int innerRectWidth = stationWidth / 2;
    int workbenchRadius = 20;

    // Äußeres Rechteck (Container, ohne Sichtbarkeit)
    QGraphicsRectItem *outerRect = new QGraphicsRectItem(0, 0, stationWidth, stationHeight);
    outerRect->setPos(x, y);
    outerRect->setBrush(Qt::NoBrush);
    scene->addItem(outerRect);

    // Linkes inneres Rechteck
    leftRect = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
    leftRect->setPos(0, 0);
    leftRect->setBrush(Qt::gray);  // default farbe
    scene->addItem(leftRect);

    // Rechtes inneres Rechteck
    rightRect = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
    rightRect->setPos(innerRectWidth, 0);
    rightRect->setBrush(Qt::gray);  // default farbe
    scene->addItem(rightRect);

    // Fett und schwarz setzen
    QFont boldFont("Arial", 12, QFont::Bold);

    // Beschriftung für linkes inneres Rechteck (außerhalb)
    QGraphicsTextItem* labelLeft = new QGraphicsTextItem(nameLeft, outerRect);
    labelLeft->setFont(boldFont);
    labelLeft->setDefaultTextColor(Qt::black);  // Schriftfarbe schwarz
    labelLeft->setPos(innerRectWidth-50, 100);  // Position oberhalb des linken Rechtecks
    labelLeft->setRotation(90);
    scene->addItem(labelLeft);

    // Beschriftung für rechtes inneres Rechteck (außerhalb)
    QGraphicsTextItem* labelRight = new QGraphicsTextItem(nameRight, outerRect);
    labelRight->setFont(boldFont);
    labelRight->setDefaultTextColor(Qt::black);  // Schriftfarbe schwarz
    labelRight->setPos(innerRectWidth + 50, 100);  // Position oberhalb des rechten Rechtecks
    labelRight->setRotation(90);
    scene->addItem(labelRight);

    // Werkbänke für linkes inneres Rechteck
    leftWorkbench1 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, leftRect);
    leftWorkbench1->setPos(30, 20);
    leftWorkbench1->setBrush(Qt::green);
    scene->addItem(leftWorkbench1);

    leftWorkbench2 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, leftRect);
    leftWorkbench2->setPos(30, 60);
    leftWorkbench2->setBrush(Qt::green);
    scene->addItem(leftWorkbench2);

    // Werkbänke für rechtes inneres Rechteck
    rightWorkbench1 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, rightRect);
    rightWorkbench1->setPos(30, 20);
    rightWorkbench1->setBrush(Qt::green);
    scene->addItem(rightWorkbench1);

    rightWorkbench2 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, rightRect);
    rightWorkbench2->setPos(30, 60);
    rightWorkbench2->setBrush(Qt::green);
    scene->addItem(rightWorkbench2);
}

void Station::setStateByName(const QString& name, const QColor& color) {
    if (name == leftName) {
        leftRect->setBrush(color);
    } else if (name == rightName) {
        rightRect->setBrush(color);
    }
}

void Station::setWorkbenchStateByName(const QString& name, int workbenchNumber, const QColor& color) {
    if (name == leftName) {
        if (workbenchNumber == 1) {
            leftWorkbench1->setBrush(color);
        } else if (workbenchNumber == 2) {
            leftWorkbench2->setBrush(color);
        }
    } else if (name == rightName) {
        if (workbenchNumber == 1) {
            rightWorkbench1->setBrush(color);
        } else if (workbenchNumber == 2) {
            rightWorkbench2->setBrush(color);
        }
    }
/*
    if(name == "Rohteillager 1" && color == Qt::blue) {
        if(workbenchNumber == 1) {
            rightWorkbench1->setBrush(Qt::magenta);
        }
        else if(workbenchNumber == 2) {
            rightWorkbench2->setBrush(Qt::magenta);
        }
    }
    else if(name == "Rohteillager 2" && color == Qt::blue) {
        if(workbenchNumber == 1) {
            leftWorkbench1->setBrush(Qt::magenta);
        }
        else if(workbenchNumber == 2) {
            leftWorkbench2->setBrush(Qt::magenta);
        }
    }
*/
}
