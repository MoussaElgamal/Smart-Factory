#ifndef PRODUCTIONOVERVIEW_H
#define PRODUCTIONOVERVIEW_H

#include <QGraphicsScene>
#include <QKeyEvent>
#include <QTimer>
#include <QMap>
#include <QWidget>
#include "ordermanager.h"
#include "sqlinterface.h"
#include "station.h"
#include "viewinterface.h"
#include <qlist.h>
#include <robotitem.h>
//
namespace Ui {
class ProductionOverview;
}

class ProductionOverview : public QWidget
{
    Q_OBJECT
    //

public:
    explicit ProductionOverview(QWidget *parent = nullptr,
                                OrderManager *orderManager = nullptr,
                                SqlInterface *sqlInterface = nullptr,
                                ViewInterface *viewInterface = nullptr);

    ~ProductionOverview();

signals:
    void backToMainPage();

private slots:
    void updateRobotPositions(); // Aktualisiere die Roboterpositionen
    void readCoordinates(int &x, int &y);
    void mousePressEvent(QMouseEvent *event) override;
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void wheelEvent(QWheelEvent *event) override;

    void loadRobotPositionX(const int robot, const int x);
    void loadRobotPositionY(const int robot, const int y);
    void changeState(Agent::Type type, int number, Agent::State state);
    void changeStateWb(Agent::Type type, int typeNumber, int workbenchNumber, Agent::State state);

    void on_backToMainPage_2_clicked();
    void on_resetRobot1_clicked();
    void on_resetRobot2_clicked();
    void on_resetRobot3_clicked();
    void on_resetRobot4_clicked();


    void showPositionText(Part::Position position); // Neuer Slot zum Anzeigen des Textes
    void clearTextBrowser();                        // Neuer Slot zum Leeren des TextBrowsers

signals:
    void resetRobot(int robot);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::ProductionOverview *ui;
    QGraphicsScene *scene;

    QVector<QGraphicsRectItem *> innerRects;     // Speichert die inneren Rechtecke
    QVector<QGraphicsEllipseItem *> workbenches; // Speichert die Werkbänke
    QList<Station *> stations;

    QGraphicsPolygonItem *blitzSymbol1;
    QGraphicsPolygonItem *blitzSymbol2;
    QGraphicsTextItem *label1;
    QGraphicsTextItem *label2;

    QTimer *timer;
    QTimer *timerStation;
    // Falls  später direkte Referenzen auf die Stationen oder Roboter , hier speichern
    // z.B. StationItem *station1, *station2, *chargeStation1, *chargeStation2;

    // Map für die Zuordnung von Position-Enums zu deutschen Texten
    QMap<Part::Position, QString> positionTextMap;
    QTimer *displayTimer;  // Neuer Timer für die 3-Sekunden-Anzeige
    ViewInterface *_viewInterface;
    OrderManager *orderManager;
    SqlInterface *sqlInterface;

    int _lastPrestorageNumber = 0;
    int _lastPrestorageWorkbench = 0;

    QList<RobotItem *> listOfRobotItems;

    void addStationsToScene(); // Methode zum Hinzufügen der Stationen
    QColor stateToColor(Agent::State state);

    void addLegendToScene();
    void styleResetRobotButtons();

    // Methode zur Rückgabe der Übersetzung basierend auf der Position
    QString getPositionText(Part::Position position) const;
};

#endif // PRODUCTIONOVERVIEW_H
