#ifndef ROBOTITEM_H
#define ROBOTITEM_H

#include <QColor>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QObject>
#include <QTimer>
#include "agent.h"
#include "ordermanager.h"
#include "sqlinterface.h"
#include "viewinterface.h"

//
class RobotItem : public QGraphicsObject
{
    Q_OBJECT // Makro für Signal-Slot-Mechanismus

public:
    explicit RobotItem(QColor robotColor, int robotId,
                       ViewInterface *viewInterface = nullptr);
    ~RobotItem();
    int getId() const; // Methode, um die Roboter-ID abzurufen
    void updateBatteryLevel2(int number);

private slots:
    void changeState(Agent::Type type, int number, Agent::State state);
    void updateBatteryLevel(int number, int batteryLevel);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:

    ViewInterface *_viewInterface;
    OrderManager *orderManager;
    SqlInterface *sqlInterface;

    QColor stateToColor(Agent::State state);

    //test
    int getBatteryState(int n);
    QColor color;
    int id; // Roboter-ID
    void showRobotInfo();
    QString getStateString(Agent::State state);
    int _batteryLevel = 0;
    QTimer *blinkTimer; // Timer für das Blinken
    bool blinkState;    // Steuerung des Blinkzustands (Ein/Aus)
};

#endif //ROBOTITEM_H

/*
class RobotItem : public QGraphicsItem
{
public:
    RobotItem(QColor color);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    //int posX, posY;
    QColor color; // Speichert die Farbe des Roboters
    void showRobotInfo();
};

#endif // ROBOTITEM_H
*/
