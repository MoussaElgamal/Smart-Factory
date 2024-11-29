#include "productionoverview.h"
#include <QDebug>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include "dimension.h"
#include "station.h"
#include "stationitem.h"
#include "ui_productionoverview.h"
#include <iostream>
#include <QColor>
//

ProductionOverview::ProductionOverview(QWidget *parent,
                                       OrderManager *orderManager,
                                       SqlInterface *sqlInterface,
                                       ViewInterface *viewInterface)
    : QWidget(parent)
    , ui(new Ui::ProductionOverview)
{
    ui->setupUi(this);
    _viewInterface = viewInterface;
    this->setFocusPolicy(Qt::StrongFocus); // Fügt den Fokus für Tastendrücke hinzu
    this->setStyleSheet("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 "
                        "#add8e6, stop:1 #87cefa);");

    QFont boldFont2("Arial", 16, QFont::Bold);

    ui->textBrowser->setFont(boldFont2);
    ui->textBrowser->setTextColor(Qt::red);
    //ui->textBrowser->setText("Maschine 1 Platz 1");
    //ui->textBrowser->setText("Rohteillager 1 Platz 1");
    // Mapping von Position-Enums zu deutschen Texten
    positionTextMap = {
        {Part::Position::robot, "Roboter"},
        {Part::Position::prestorage1place1, "Rohteillager 1 Platz 1"},
        {Part::Position::prestorage1place2, "Rohteillager 1 Platz 2"},
        {Part::Position::prestorage2place1, "Rohteillager 2 Platz 1"},
        {Part::Position::prestorage2place2, "Rohteillager 2 Platz 2"},
        {Part::Position::machine1place1, "Maschine 1 Platz 1"},
        {Part::Position::machine1place2, "Maschine 1 Platz 2"},
        {Part::Position::machine2place1, "Maschine 2 Platz 1"},
        {Part::Position::machine2place2, "Maschine 2 Platz 2"},
        {Part::Position::machine3place1, "Maschine 3 Platz 1"},
        {Part::Position::machine3place2, "Maschine 3 Platz 2"},
        {Part::Position::machine4place1, "Maschine 4 Platz 1"},
        {Part::Position::machine4place2, "Maschine 4 Platz 2"},
        {Part::Position::finalstorage1place1, "Fertigungslager 1 Platz 1"},
        {Part::Position::finalstorage1place2, "Fertigungslager 1 Platz 2"},
        {Part::Position::finalstorage2place1, "Fertigungslager 2 Platz 1"},
        {Part::Position::finalstorage2place2, "Fertigungslager 2 Platz 2"},
        {Part::Position::pickup, "Abholpunkt"},
        {Part::Position::placedown, "Abgabepunkt"},
        {Part::Position::prestorage1rfid, "Rohteillager 1 RFID"},
        {Part::Position::prestorage2rfid, "Rohteillager 2 RFID"},
        {Part::Position::machine1rfid, "Maschine 1 RFID"},
        {Part::Position::machine2rfid, "Maschine 2 RFID"},
        {Part::Position::machine3rfid, "Maschine 3 RFID"},
        {Part::Position::machine4rfid, "Maschine 4 RFID"},
        {Part::Position::finalstorage1rfid, "Fertigungslager 1 RFID"},
        {Part::Position::finalstorage2rfid, "Fertigungslager 2 RFID"},
        {Part::Position::undefined, "Unbekannt"},
        {Part::Position::customer, "Kunde"}
    };


    // Timer initialisieren
    displayTimer = new QTimer(this);
    displayTimer->setInterval(5000); // 5 Sekunden

    // Verbindung herstellen: Timer abgelaufen -> Text im TextBrowser löschen
    connect(displayTimer, &QTimer::timeout, this, &ProductionOverview::clearTextBrowser);

    // Verbindung herstellen: Position ändern -> Text anzeigen
    //connect(viewInterface, &ViewInterface::showPositionText, this, &ProductionOverview::showPositionText);
    connect(viewInterface,
            &ViewInterface::loadRobotPositionX,
            this,
            &ProductionOverview::loadRobotPositionX);
    connect(viewInterface,
            &ViewInterface::loadRobotPositionY,
            this,
            &ProductionOverview::loadRobotPositionY);

    connect(viewInterface, &ViewInterface::changeState, this, &ProductionOverview::changeState);
    connect(viewInterface, &ViewInterface::changeStateWb, this, &ProductionOverview::changeStateWb);
    if (viewInterface != nullptr) {
        connect(this, &ProductionOverview::resetRobot, viewInterface, &ViewInterface::resetRobotBumper);
    }
    this->setMouseTracking(true);             // Für das Hauptfenster
    ui->graphicsView->setMouseTracking(true); // Für die QGraphicsView

    // Szene erstellen und in die View einfügen
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    ui->backToMainPage_2->setIconSize(QSize(100, 50));
    QIcon iconMainmenu = (QPixmap("../../Hauptmenü.png"));
    ui->backToMainPage_2->setIcon(iconMainmenu);
    ui->graphicsView->scale(0.9, 0.9);
    // Die Stationen zu der bestehenden Szene hinzufügen
    addStationsToScene();

    //Legends
    addLegendToScene();

    // Scrollbars deaktivieren
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Szene Dimensionen festlegen
    ui->graphicsView->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT); // Vergrößerte Größe für mehr Platz
    scene->setSceneRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT);      // Zentrierte Szene

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    //ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->setInteractive(true); // Setze die View als interaktiv

    // Farben für Roboter definieren
    QColor colors[4] = {Qt::green, Qt::cyan, Qt::magenta, Qt::yellow};

    // Roboter hinzufügen und positionieren
    for (int i = 0; i < 4; i++) {
        RobotItem *robot = new RobotItem(colors[i],
                                         i + 1,
                                         _viewInterface); // Setze eine eindeutige ID für jeden Roboter
        robot->setPos(((0 / SCALE_FACTOR_REAL) * SCALE_FACTOR),
                      SCENE_HEIGHT
                          - (((0 / SCALE_FACTOR_REAL)
                              * SCALE_FACTOR))); // Initial Position der Roboter
        robot->setScale(1);                      // Roboter vergrößern für bessere Sichtbarkeit
        scene->addItem(robot);
        listOfRobotItems.append(robot);
    }
    // Fertigungsstationen hinzufügen
    StationItem *station1 = new StationItem(Qt::gray,
                                            "S",
                                            4,
                                            STATION_WIDTH + TEST_OFFSET_X,
                                            STATION_HEIGHT);
    station1->setPos(STATION1_X, SCENE_HEIGHT - STATION1_Y);
    scene->addItem(station1);

    StationItem *station2 = new StationItem(Qt::gray,
                                            "S",
                                            3,
                                            STATION_WIDTH + TEST_OFFSET_X,
                                            STATION_HEIGHT);
    station2->setPos(STATION2_X, SCENE_HEIGHT - STATION2_Y);
    scene->addItem(station2);

    StationItem *station3 = new StationItem(Qt::gray,
                                            "S",
                                            2,
                                            STATION_WIDTH + TEST_OFFSET_X,
                                            STATION_HEIGHT);
    station3->setPos(STATION3_X, SCENE_HEIGHT - STATION3_Y);
    scene->addItem(station3);

    StationItem *station4 = new StationItem(Qt::gray, "S", 1, STATION_WIDTH, STATION_HEIGHT);
    station4->setPos(STATION4_X, SCENE_HEIGHT - STATION4_Y);
    scene->addItem(station4);

    // Ladestationen hinzufügen
    StationItem *chargeStation1 = new StationItem(Qt::gray,
                                                  "L",
                                                  1,
                                                  CHARGE_STATION_WIDTH,
                                                  CHARGE_STATION_HEIGHT);
    chargeStation1->setPos(CHARGE_STATION1_X, SCENE_HEIGHT - CHARGE_STATION1_Y);
    scene->addItem(chargeStation1);

    StationItem *chargeStation2 = new StationItem(Qt::gray,
                                                  "L",
                                                  2,
                                                  CHARGE_STATION_WIDTH,
                                                  CHARGE_STATION_HEIGHT);
    chargeStation2->setPos(CHARGE_STATION2_X, SCENE_HEIGHT - CHARGE_STATION2_Y);
    scene->addItem(chargeStation2);

    // Ladestation 1 Blitzsymbol hinzufügen
    QPolygonF blitz;
    blitz << QPointF(0, 0) << QPointF(10, 30) << QPointF(5, 30) << QPointF(15, 60) << QPointF(5, 60)
          << QPointF(20, 90) << QPointF(0, 50);

    // Ladestation 1 Blitzsymbol hinzufügen
    blitzSymbol1 = new QGraphicsPolygonItem(blitz);
    blitzSymbol1->setPos(CHARGE_STATION1_X,
                         SCENE_HEIGHT - CHARGE_STATION1_Y - 150); // Über der Ladestation
    blitzSymbol1->setBrush(Qt::green);                            // Standardfarbe Gelb
    scene->addItem(blitzSymbol1);

    // Label für Ladestation 1 hinzufügen
    label1 = new QGraphicsTextItem("L1");
    label1->setPos(CHARGE_STATION1_X, SCENE_HEIGHT - CHARGE_STATION1_Y - 60); // Über der Ladestation
    // Fett und schwarz setzen
    //
    QFont boldFont("Arial", 12, QFont::Bold);
    label1->setFont(boldFont);
    label1->setDefaultTextColor(Qt::black); // Setze die Schriftfarbe auf Schwarz
    scene->addItem(label1);

    // Ladestation 2 Blitzsymbol hinzufügen
    blitzSymbol2 = new QGraphicsPolygonItem(blitz);
    blitzSymbol2->setPos(CHARGE_STATION2_X,
                         SCENE_HEIGHT - CHARGE_STATION2_Y - 150); // Über der Ladestation
    blitzSymbol2->setBrush(Qt::green);                            // Standardfarbe Gelb
    scene->addItem(blitzSymbol2);

    // Label für Ladestation 2 hinzufügen
    label2 = new QGraphicsTextItem("L2");
    label2->setPos(CHARGE_STATION2_X, SCENE_HEIGHT - CHARGE_STATION2_Y - 60); // Über der Ladestation
    label2->setFont(boldFont);
    label2->setDefaultTextColor(Qt::black);
    scene->addItem(label2);

    // Timer zur Aktualisierung der Roboterpositionen starten
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ProductionOverview::updateRobotPositions);
    timer->start(50); // Aktualisierung alle 100 ms

    // QAction für Zoom-Zurücksetzen erstellen
    QAction *resetZoomAction = new QAction("Reset Zoom", this);

    // Shortcut zuweisen (Strg + R)
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

    // Aktion mit der Funktion zum Zurücksetzen des Zooms verbinden
    connect(resetZoomAction, &QAction::triggered, this, &ProductionOverview::resetZoom);

    // Füge die Aktion zum Menü hinzu, falls du ein Menü hast
    //ui->menubar->addAction(resetZoomAction);

    // Optional: Die Aktion kann auch global funktionieren, ohne Menü
    this->addAction(resetZoomAction);

    // Formatierung der Buttons
    QPushButton *resetRobotButtons[] = {ui->resetRobot1,
                                        ui->resetRobot2,
                                        ui->resetRobot3,
                                        ui->resetRobot4};
    QFont font;
    font.setBold(true);

    for (QPushButton *button : resetRobotButtons) {
        button->setFont(font);
        button->setStyleSheet("color: black;");
    }
}

// Methode zur Rückgabe der deutschen Übersetzung
QString ProductionOverview::getPositionText(Part::Position position) const {
    return positionTextMap.value(position, "Unbekannte Position");
}
// Slot, um den Text für 3 Sekunden anzuzeigen
void ProductionOverview::showPositionText(Part::Position position) {
    QString translatedText = getPositionText(position);
    ui->textBrowser->setText(translatedText);  // Text im TextBrowser setzen

    displayTimer->start();  // Timer starten, um Text nach 3 Sekunden zu löschen
}

// Slot, um den TextBrowser zu leeren
void ProductionOverview::clearTextBrowser() {
    ui->textBrowser->clear();  // Text aus dem TextBrowser löschen
    displayTimer->stop();      // Timer stoppen, um unnötiges Wiederholen zu verhindern
}
void ProductionOverview::keyPressEvent(QKeyEvent *event)
{
    // Mappe Tasten auf Farbwerte
    static const QMap<int, QString> colorMap = {
        {Qt::Key_W, "#ffffff"}, // Weiß
        {Qt::Key_B, "#0000ff"}, // Blau
        {Qt::Key_R, "#ff0000"}, // Rot
        {Qt::Key_G, "#008000"}, // Grün
        {Qt::Key_Y, "#ffff00"}, // Gelb
        {Qt::Key_P, "#800080"}, // Lila
        {Qt::Key_O, "#ffa500"}, // Orange
        {Qt::Key_C, "#00ffff"}, // Cyan
        {Qt::Key_M, "#ff00ff"}, // Magenta
        {Qt::Key_D, "default"}  // D für Standardfarbe
    };

    if (colorMap.contains(event->key())) {
        QString color1 = colorMap[event->key()];

        if (color1 == "default") {
            // Setze den Hintergrund auf die Standardfarbe zurück
            this->setStyleSheet("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 "
                                "#add8e6, stop:1 #87cefa);");
        } else {
            // Erzeuge eine dunklere Version von color1 für den Farbverlauf
            QColor baseColor(color1);
            QColor darkerColor = baseColor.darker(150); // Wert über 100 dunkelt die Farbe ab

            QString color2 = darkerColor.name();
            this->setStyleSheet(QString("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
                                        "y2:1, stop:0 %1, stop:1 %2);")
                                    .arg(color1, color2));
        }
    } else {
        // Für nicht definierte Tasten das Standardverhalten beibehalten
        QWidget::keyPressEvent(event);
    }
}

/*
void ProductionOverview::addStationsToScene()
{
    // Größe und Position der äußeren Rechtecke (Stationen)
    int stationWidth = 200;
    int stationHeight = 100;
    int innerRectWidth = stationWidth / 2;
    int workbenchRadius = 20;
    int spacing = 20; // Abstand zwischen den äußeren Rechtecken und der unteren Station

    // Die y-Position der äußeren Rechtecke (leicht oberhalb der bestehenden Stationen)
    int upperYPos = 200; // Oberhalb der unteren Stationen

    // Farben für Demonstration (wird später durch Signale geändert)
    QColor initialColor = Qt::lightGray;

    // Namen der inneren Rechtecke (Stationen)
    QStringList innerRectNames = {
        "Fertigungslager 2", "Fertigungslager 1",
        "Maschine 4", "Maschine 3",
        "Maschine 2", "Maschine 1",
        "Rohteillager 2", "Rohteillager 1"
    };

    int nameIndex = 0; // Zum Durchlaufen der Namen

    // Positionierung der neuen Stationen über den vorhandenen Stationen
    QList<double> stationXPositions = {STATION1_X, STATION2_X, STATION3_X, STATION4_X}; // Von links nach rechts

    // Erstelle 4 neue Stationen (äußere Rechtecke über den unteren Stationen)
    for (int i = 0; i < 4; i++) {
        // Mittige Position für die äußeren Rechtecke basierend auf der unteren Station
        int currentXPos = stationXPositions[i] - stationWidth / 2;

        // Äußeres Rechteck (Station)
        QGraphicsRectItem *outerRect = new QGraphicsRectItem(0, 0, stationWidth, stationHeight);
        outerRect->setPos(currentXPos, upperYPos); // Position des äußeren Rechtecks über den unteren Stationen
        outerRect->setBrush(Qt::NoBrush);
        scene->addItem(outerRect);

        // Inneres Rechteck 1 (links, z.B. "Fertigungslager 2")
        QGraphicsRectItem *innerRect1 = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
        innerRect1->setPos(0, 0); // Links im äußeren Rechteck
        innerRect1->setBrush(Qt::green);
        scene->addItem(innerRect1);
        innerRects.append(innerRect1); // Speichere für spätere Updates

        // Beschriftung für inneres Rechteck 1
        QGraphicsTextItem *label1 = new QGraphicsTextItem(innerRectNames[nameIndex], innerRect1);
        label1->setPos(10, 10); // Position der Beschriftung innerhalb des Rechtecks
        scene->addItem(label1);
        nameIndex++; // Nächster Name

        // Werkbänke (Ellipsen) in innerem Rechteck 1
        QGraphicsEllipseItem *workbench1 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, innerRect1);
        workbench1->setPos(30, 20); // Obere Werkbank
        scene->addItem(workbench1);
        workbenches.append(workbench1);

        QGraphicsEllipseItem *workbench2 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, innerRect1);
        workbench2->setPos(30, 60); // Untere Werkbank
        scene->addItem(workbench2);
        workbenches.append(workbench2);

        // Inneres Rechteck 2 (rechts, z.B. "Fertigungslager 1")
        QGraphicsRectItem *innerRect2 = new QGraphicsRectItem(0, 0, innerRectWidth, stationHeight, outerRect);
        innerRect2->setPos(innerRectWidth, 0); // Rechts im äußeren Rechteck
        innerRect2->setBrush(initialColor);
        scene->addItem(innerRect2);
        innerRects.append(innerRect2); // Speichere für spätere Updates

        // Beschriftung für inneres Rechteck 2
        QGraphicsTextItem *label2 = new QGraphicsTextItem(innerRectNames[nameIndex], innerRect2);
        label2->setPos(10, 10); // Position der Beschriftung innerhalb des Rechtecks
        scene->addItem(label2);
        nameIndex++; // Nächster Name

        // Werkbänke (Ellipsen) in innerem Rechteck 2
        QGraphicsEllipseItem *workbench3 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, innerRect2);
        workbench3->setPos(30, 20); // Obere Werkbank
        scene->addItem(workbench3);
        workbenches.append(workbench3);

        QGraphicsEllipseItem *workbench4 = new QGraphicsEllipseItem(0, 0, workbenchRadius, workbenchRadius, innerRect2);
        workbench4->setPos(30, 60); // Untere Werkbank
        scene->addItem(workbench4);
        workbenches.append(workbench4);
    }
}


void ProductionOverview::updateInnerRectState(int stationIndex, const QColor &color)
{
    if (stationIndex >= 0 && stationIndex < innerRects.size()) {
        innerRects[stationIndex]->setBrush(color);
    }
}

void ProductionOverview::updateWorkbenchState(int stationIndex, int workbenchIndex, const QColor &color)
{
    int index = stationIndex * 2 + workbenchIndex;
    if (index >= 0 && index < workbenches.size()) {
        workbenches[index]->setBrush(color);
    }
}
*/

void ProductionOverview::addStationsToScene()
{
    int spacing = 90;
    const int y = 100;
    // Beispiel für die Positionierung (X-Werte müssen für jede Station angepasst werden)
    stations.append(
        new Station("Fertigungslager 2", "Fertigungslager 1", STATION1_X - spacing, y, scene));
    stations.append(new Station("Maschine 4", "Maschine 3", STATION2_X - spacing, y, scene));
    stations.append(new Station("Maschine 2", "Maschine 1", STATION3_X - spacing, y, scene));
    stations.append(new Station("Rohteillager 2", "Rohteillager 1", STATION4_X - spacing, y, scene));
}

void ProductionOverview::changeState(Agent::Type type, int number, Agent::State state)
{
    QColor color = stateToColor(state);
    QString name;

    if (type == Agent::Type::chargingPlace) {
        // Blitzfarbe basierend auf dem State ändern
        if (number == 1) {
            blitzSymbol1->setBrush(color); // Ändere die Blitzfarbe von Ladestation 1
        } else if (number == 2) {
            blitzSymbol2->setBrush(color); // Ändere die Blitzfarbe von Ladestation 2
        }
        return; // Da wir uns nur für chargingPlace interessieren, können wir hier beenden
    }

    // Bestimme den Namen der inneren Rechtecke basierend auf Typ und Nummer
    /*if (type == Agent::Type::machine) {
        name = "Maschine " + QString::number(number);
    } else if (type == Agent::Type::prestorage) {
        name = "Rohteillager " + QString::number(number);
    } else if (type == Agent::Type::finalstorage) {
        name = "Fertigungslager " + QString::number(number);
    }
   */
    // Suche die Station und setze die Farbe basierend auf dem Namen
    for (auto &station : stations) {
        station->setStateByName(name, color);
    }
}

void ProductionOverview::changeStateWb(Agent::Type type,
                                       int typeNumber,
                                       int workbenchNumber,
                                       Agent::State state)
{
    QColor color = stateToColor(state);
    QString name;

    // Bestimme den Namen der inneren Rechtecke basierend auf Typ und Nummer
    if (type == Agent::Type::machine) {
        name = "Maschine " + QString::number(typeNumber);
    } else if (type == Agent::Type::prestorage) {
        name = "Rohteillager " + QString::number(typeNumber);
        if (state == Agent::State::allocated) {
            if (typeNumber == 1) {
                if (workbenchNumber == 1) {
                    showPositionText(Part::Position::prestorage1place1);
                }
                if (workbenchNumber == 2) {
                    showPositionText(Part::Position::prestorage1place2);
                }
            }
            if (typeNumber == 2) {
                if (workbenchNumber == 1) {
                    showPositionText(Part::Position::prestorage2place1);
                }
                if (workbenchNumber == 2) {
                    showPositionText(Part::Position::prestorage2place2);
                }
            }
        }
    } else if (type == Agent::Type::finalstorage) {
        name = "Fertigungslager " + QString::number(typeNumber);
    }

    // Suche die Station und setze die Werkbankfarbe basierend auf dem Namen und der Werkbanknummer
    for (auto &station : stations) {
        station->setWorkbenchStateByName(name, workbenchNumber, color);
    }
}

QColor ProductionOverview::stateToColor(Agent::State state)
{
    switch (state) {
    case Agent::State::error:
        return Qt::red;
    case Agent::State::free:
        return Qt::green;
    case Agent::State::reserved:
        return Qt::yellow;
    case Agent::State::allocated:
        return Qt::blue;
    default:
        return Qt::lightGray;
    }
}

/*void ProductionOverview::addLegendToScene()
{
    // Legendentitel
    QGraphicsTextItem *legendTitle = new QGraphicsTextItem("Legende:");
    legendTitle->setPos(60, 5);                  // Position neben der Szene
    legendTitle->setDefaultTextColor(Qt::black); // Farbe auf Schwarz setzen
    QFont titleFont;
    titleFont.setBold(true);
    legendTitle->setFont(titleFont); // Schriftart auf Bold setzen
    scene->addItem(legendTitle);

    // Legendeninhalte
    struct LegendItem
    {
        QString text;
        QColor color;
    };

    LegendItem legendItems[] = {{"Error", Qt::red},
                                {"Free", Qt::green},
                                {"Reserved", Qt::yellow},
                                {"Allocated", Qt::blue},
                                {"Default", Qt::lightGray}};

    int yOffset = 30; // Abstand zwischen den Legendeninhalten
    for (const auto &item : legendItems) {
        // Farbe
        QGraphicsRectItem *colorBox = new QGraphicsRectItem(0, 0, 20, 20);
        colorBox->setBrush(QBrush(item.color));
        colorBox->setPos(70, yOffset);
        scene->addItem(colorBox);

        // Text
        QGraphicsTextItem *label = new QGraphicsTextItem(item.text);
        label->setPos(100, yOffset);
        label->setDefaultTextColor(Qt::black); // Textfarbe auf Schwarz setzen
        QFont labelFont;
        labelFont.setBold(true);
        label->setFont(labelFont); // Schriftart auf Bold setzen
        scene->addItem(label);

        yOffset += 30; // Platz für den nächsten Eintrag
    }
}*/

void ProductionOverview::addLegendToScene()
{
    // Legendentitel
    QGraphicsTextItem *legendTitle = new QGraphicsTextItem("Legende:");
    legendTitle->setPos(60, 5);                  // Position neben der Szene
    legendTitle->setDefaultTextColor(Qt::black); // Farbe auf Schwarz setzen
    QFont titleFont;
    titleFont.setBold(true);
    legendTitle->setFont(titleFont); // Schriftart auf Bold setzen
    scene->addItem(legendTitle);

    // Legendeninhalte
    struct LegendItem
    {
        QString text;
        QColor color;
    };

    LegendItem legendItems[] = {
        {"Error", Qt::red},
        {"Free", Qt::green},
        {"Reserved", Qt::yellow},
        {"Allocated", Qt::blue},
        {"Default", Qt::lightGray},
        // Leere Zeile für Abstand
        {"", QColor()},
        {"Roboter 1", Qt::green},
        {"Roboter 2", Qt::cyan},
        {"Roboter 3", Qt::magenta},
        {"Roboter 4", Qt::yellow}
    };

    int yOffset = 30; // Abstand zwischen den Legendeninhalten
    for (const auto &item : legendItems) {
        // Lücke hinzufügen (nur wenn Text leer ist)
        if (item.text.isEmpty()) {
            yOffset += 200; // Extra Platz für die Lücke
            continue;
        }

        // Farbe
        QGraphicsRectItem *colorBox = new QGraphicsRectItem(0, 0, 20, 20);
        colorBox->setBrush(QBrush(item.color));
        colorBox->setPos(70, yOffset);
        scene->addItem(colorBox);

        // Text
        QGraphicsTextItem *label = new QGraphicsTextItem(item.text);
        label->setPos(100, yOffset);
        label->setDefaultTextColor(Qt::black); // Textfarbe auf Schwarz setzen
        QFont labelFont;
        labelFont.setBold(true);
        label->setFont(labelFont); // Schriftart auf Bold setzen
        scene->addItem(label);

        yOffset += 30; // Platz für den nächsten Eintrag
    }
}

ProductionOverview::~ProductionOverview()
{
    delete ui;
}

// Zoom-Funktionen
void ProductionOverview::zoomIn()
{
    ui->graphicsView->scale(1.2, 1.2); // Vergrößern
}

void ProductionOverview::zoomOut()
{
    ui->graphicsView->scale(0.8, 0.8); // Verkleinern
}

// wheelEvent überschreiben
void ProductionOverview::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn(); // Vergrößern bei Scrollen nach oben
    } else {
        zoomOut(); // Verkleinern bei Scrollen nach unten
    }
}

void ProductionOverview::loadRobotPositionX(const int robot, const int x)
{
    if (robot >= 1 && robot <= 4) {
        QPointF pos = listOfRobotItems.at(robot - 1)->pos();
        pos.setX(((x / SCALE_FACTOR_REAL) * SCALE_FACTOR));
        listOfRobotItems.at(robot - 1)->setPos(pos);
        scene->update();
    }
}

void ProductionOverview::loadRobotPositionY(const int robot, const int y)
{
    if (robot >= 1 && robot <= 4) {
        QPointF pos = listOfRobotItems.at(robot - 1)->pos();
        pos.setY(SCENE_HEIGHT - (((y / SCALE_FACTOR_REAL) * SCALE_FACTOR)));
        listOfRobotItems.at(robot - 1)->setPos(pos);
        scene->update();
    }
}

void ProductionOverview::resetZoom()
{
    ui->graphicsView->resetTransform(); // Setzt den Zoom auf Standardgröße zurück
}

void ProductionOverview::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // Wenn das Fenster maximiert wird
        if (this->isMaximized()) {
            ui->graphicsView->resetTransform();  // Transform zurücksetzen
            ui->graphicsView->scale(1.15, 1.15); // Skalierung auf 1.5 setzen
        }
        // Wenn das Fenster zum normalen Zustand zurückkehrt
        else if (this->windowState() == Qt::WindowNoState) {
            ui->graphicsView->resetTransform(); // Transform auf Standardgröße zurücksetzen
        }
    }
    //QMainWindow::changeEvent(event);  // Basisimplementierung aufrufen
}

void ProductionOverview::hideEvent(QHideEvent *event)
{
    // Das Fenster wurde minimiert
    if (this->isMinimized()) {
        ui->graphicsView->resetTransform(); // Transform zurücksetzen
    }
    //QMainWindow::hideEvent(event);  // Basisimplementierung aufrufen
}

//test

void ProductionOverview::mousePressEvent(QMouseEvent *event)
{
    // Überprüfe, ob die linke Maustaste gedrückt wurde
    if (event->button() == Qt::LeftButton) {
        // Hol die Mauskoordinaten
        QPointF scenePos = ui->graphicsView->mapToScene(event->pos());

        // Ausgabe der Mauskoordinaten in der Konsole
        //qDebug() << "Maus bewegt sich zu: x =" << (scenePos.x())*10 << ", y =" << 10*(600-scenePos.y());
        // AS: musste auskommentieren weil sonst nix mehr lesbar

        // Optional: Rufe die Standardimplementierung auf
        //QMainWindow::mousePressEvent(event);
    }
}

void ProductionOverview::readCoordinates(int &x, int &y)
{
    // Eingabe der Werte über die Konsole
    std::cout << "Geben Sie den Wert für x ein: ";
    std::cin >> x;

    std::cout << "Geben Sie den Wert für y ein: ";
    std::cin >> y;
    qDebug() << "Die Koordinaten sind:" << "x =" << x << ", y =" << y;
}

void ProductionOverview::updateRobotPositions()
{
    foreach (QGraphicsItem *item, scene->items()) {
        RobotItem *robot = dynamic_cast<RobotItem *>(item);
        if (robot) {
            int robotId = robot->getId();
            robot->updateBatteryLevel2(robotId);
        }
    }
    scene->update();

    /*
    foreach (QGraphicsItem *item, scene->items()) {
        RobotItem *robot = dynamic_cast<RobotItem*>(item);
        if (robot) {



            int robotId = robot->getId();  // Hol die Roboter-ID
            int x, y;
*/
    /*// Call the takeOrder function to send the order details
            if (viewInterface != nullptr) {
                try {
                    //qDebug() << "Vor "; // AS: musste auskommentieren weil sonst nix mehr lesbar
                    bool valid = viewInterface->getRobotPosition(robotId-1, x, y);
                    if (valid) {
                        //y = 6000 - y;
                        qreal newX = x;
                        qreal newY = y;
                        //qDebug() << "Robot" <<robotId;
                        //qDebug() << "x:" <<newX;
                        //qDebug() << "y:" <<newY;
                        // AS: musste auskommentieren weil sonst nix mehr lesbar

                        robot->setPos(newX/10, 550-(newY/10));
                    }
                    // Ladezustand leicht ändern, um es dynamisch zu simulieren
                    int delta = (rand() % 3) - 1;  // Ändere den Ladezustand zufällig um -1, 0 oder 1
                    robot->updateBatteryLevel(delta);
                    //qDebug() << "Nach "; // AS: musste auskommentieren weil sonst nix mehr lesbar

                } catch (const std::exception &e) {
                    qDebug() << "Exception caught in : " << e.what();
                } catch (...) {
                    qDebug() << "Unknown exception caught in !";
                }
            } else {
                qDebug() << "viewInterface ist null!";
            }
            //bool valid = viewInterface.getRobotPosition(robotId, x, y);

            */
    /*
            //test
            bool valid2 = true;
            if(robotId == 1)
            {
                x = 1200;
                y = 2800;
            }
            else if(robotId == 2)
            {
                // Aktuelle Position abrufen
                QPointF currentPos = robot->pos();

                // Direktes Bewegen des Roboters um eine kleine Anzahl von Einheiten (z.B. 50 und 10)
                qreal newX = currentPos.x() - 1;  // Bewege 5 Einheiten nach rechts
                qreal newY = currentPos.y() - 1;  // Bewege 1 Einheit nach unten

                robot->setPos(newX, newY);


                // Ladezustand leicht ändern, um es dynamisch zu simulieren
                int delta = (rand() % 3) - 1;  // Ändere den Ladezustand zufällig um -1, 0 oder 1
                robot->updateBatteryLevel(delta);

                continue;
            }
            else if(robotId == 3)
            {
                x = 6200;
                y = 3420;
            }
            else if(robotId == 4)
            {
                x = 2100;
                y = 3200;
            }
            qreal newX = x;
            qreal newY = y;
            robot->setPos(((newX/SCALE_FACTOR_REAL)*SCALE_FACTOR), SCENE_HEIGHT-(((newY/SCALE_FACTOR_REAL)*SCALE_FACTOR)));

            // Ladezustand leicht ändern, um es dynamisch zu simulieren
            int delta = (rand() % 3) - 1;  // Ändere den Ladezustand zufällig um -1, 0 oder 1
            robot->updateBatteryLevel(delta);

        }
    }
    scene->update();
*/
}

/*void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Neue Fenstergröße
    QSize newSize = event->size();

    // Berechne die Skalierungsfaktoren relativ zur Grafikansicht
    qreal scaleX = newSize.width() / static_cast<qreal>(ui->graphicsView->width());
    qreal scaleY = newSize.height() / static_cast<qreal>(ui->graphicsView->height());

    // Stelle sicher, dass die Szene im sichtbaren Bereich bleibt
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}*/

/* // Funktion zur Aktualisierung der Roboterpositionen
void MainWindow::updateRobotPositions()
{
    // Bewege nur die Roboter, nicht die Stationen
    foreach (QGraphicsItem *item, scene->items()) {
        RobotItem *robot = dynamic_cast<RobotItem*>(item);
        if (robot) {
            qreal newX = robot->x() + (rand() % 10 - 5);  // Zufällige Bewegung auf der X-Achse
            qreal newY = robot->y() + (rand() % 10 - 5);  // Zufällige Bewegung auf der Y-Achse
            robot->setPos(newX, newY);
        }
    }
}
*/

/*#include "mainwindow.h"
#include <QGraphicsRectItem>
#include "robotitem.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Fenstertitel setzen
    this->setWindowTitle("Robot Control");

    // Szene erstellen und in die View einfügen
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Scrollbars deaktivieren
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Szene Dimensionen festlegen
    ui->graphicsView->setFixedSize(1000, 600);  // Vergrößerte Größe für mehr Platz
    scene->setSceneRect(0, 0, 1000, 600);  // Zentrierte Szene
    //scene->setSceneRect(0, 0, 1000, 500);  // Angepasste Szene Größe

    // View an die Szene anpassen
    //ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // Farben für Roboter definieren
    QColor colors[4] = {Qt::red, Qt::green, Qt::blue, Qt::yellow};

    // Roboter hinzufügen und positionieren
    for (int i = 0; i < 4; i++) {
        RobotItem *robot = new RobotItem(colors[i]);
        robot->setPos(900+i, 250);  // Position der Roboter
        robot->setScale(1);  // Roboter vergrößern für bessere Sichtbarkeit3
        scene->addItem(robot);
    }

    // Stationen als Rechtecke zeichnen
    QGraphicsRectItem *station1 = scene->addRect(96, 600-260, 30, 60, QPen(Qt::black), QBrush(Qt::gray));
    QGraphicsTextItem *text1 = scene->addText("S1");
    text1->setPos(92, 600-260);
    text1->setScale(2);

    QGraphicsRectItem *station2 = scene->addRect(218, 600-260, 30, 60, QPen(Qt::black), QBrush(Qt::gray));
    QGraphicsTextItem *text2 = scene->addText("S2");
    text2->setPos(214, 600-260);
    text2->setScale(2);

    QGraphicsRectItem *station3 = scene->addRect(339, 600-260, 30, 60, QPen(Qt::black), QBrush(Qt::gray));
    QGraphicsTextItem *text3 = scene->addText("S3");
    text3->setPos(335, 600-260);
    text3->setScale(2);

    QGraphicsRectItem *station4 = scene->addRect(461, 600-260, 30, 60, QPen(Qt::black), QBrush(Qt::gray));
    QGraphicsTextItem *text4 = scene->addText("S4");
    text4->setPos(456, 600-260);
    text4->setScale(2);

    // Ladestationen als Rechtecke zeichnen
    QGraphicsRectItem *chargeStation1 = scene->addRect(620, 600-342, 40, 40, QPen(Qt::black), QBrush(Qt::blue));
    QGraphicsTextItem *chargeText1 = scene->addText("L1");
    chargeText1->setPos(620, 600-342);
    chargeText1->setScale(2);

    QGraphicsRectItem *chargeStation2 = scene->addRect(769, 600-342, 40, 40, QPen(Qt::black), QBrush(Qt::blue));
    QGraphicsTextItem *chargeText2 = scene->addText("L2");
    chargeText2->setPos(769, 600-342);
    chargeText2->setScale(2);

    // Timer zur Aktualisierung der Roboterpositionen starten
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateRobotPositions);
    timer->start(100);  // Aktualisierung alle 100 ms
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Funktion zur Aktualisierung der Roboterpositionen
void MainWindow::updateRobotPositions()
{
    // Bewege nur die Roboter, nicht die Stationen
    foreach (QGraphicsItem *item, scene->items()) {
        RobotItem *robot = dynamic_cast<RobotItem*>(item);
        if (robot) {
            qreal newX = robot->x() + (rand() % 10 - 5);  // Zufällige Bewegung auf der X-Achse
            qreal newY = robot->y() + (rand() % 10 - 5);  // Zufällige Bewegung auf der Y-Achse
            robot->setPos(newX, newY);

        }
    }
}
*/
/* #include "mainwindow.h"
#include "robotitem.h"
#include "stationitem.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setze den Fenstertitel
    this->setWindowTitle("Robot Control");

    // Erstelle die Szene und füge sie in die Ansicht ein
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Auto-Scroll der GraphicsView deaktivieren, damit sich der Frame nicht bewegt
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Setze einen größeren festen Ansichtsbereich (Viewport)
    ui->graphicsView->setFixedSize(1000, 600);  // Vergrößerte Größe für mehr Platz
    scene->setSceneRect(-500, -300, 1000, 600);  // Zentrierte Szene

    // Farben für Roboter
    QColor colors[4] = {Qt::red, Qt::green, Qt::blue, Qt::yellow};

    // Füge die Roboter zur Szene hinzu und positioniere sie relativ zur Mitte
    for (int i = 0; i < 4; i++) {
        RobotItem *robot = new RobotItem(colors[i]);
        robot->setPos(-200 + i * 50, 50);  // Startposition der Roboter relativ zur Mitte
        scene->addItem(robot);
    }

    // Füge die 4 Stationen hinzu und positioniere sie nach unten links
    for (int i = 0; i < 4; i++) {
        StationItem *station = new StationItem(Qt::gray, "Station " + QString::number(i+1));
        station->setPos(-400 + i * 100, 10);  // Nach unten links verschoben
        scene->addItem(station);
    }

    // Füge die 2 Ladestationen hinzu und positioniere sie oben rechts
    for (int i = 0; i < 2; i++) {
        StationItem *chargeStation = new StationItem(Qt::blue, "Ladestation");
        chargeStation->setPos( i * 100, -100);  // Nach oben rechts verschoben
        scene->addItem(chargeStation);
    }

    // Starte einen Timer zur Aktualisierung der Roboterpositionen
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateRobotPositions);
    timer->start(500);  // Aktualisierung alle 100 ms
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Funktion zur Aktualisierung der Roboterpositionen
void MainWindow::updateRobotPositions()
{
    // Bewege nur die Roboter, nicht die Stationen
    foreach (QGraphicsItem *item, scene->items()) {
        RobotItem *robot = dynamic_cast<RobotItem*>(item);
        if (robot) {
            qreal newX = robot->x() + (rand() % 10 - 5);  // Zufällige Bewegung auf der X-Achse
            qreal newY = robot->y() + (rand() % 10 - 5);  // Zufällige Bewegung auf der Y-Achse
            robot->setPos(newX, newY);
        }
    }
}
*/

void ProductionOverview::on_backToMainPage_2_clicked()
{
    emit backToMainPage();
}

void ProductionOverview::on_resetRobot1_clicked()
{
    emit resetRobot(1);
}

void ProductionOverview::on_resetRobot2_clicked()
{
    emit resetRobot(2);
}

void ProductionOverview::on_resetRobot3_clicked()
{
    emit resetRobot(3);
}

void ProductionOverview::on_resetRobot4_clicked()
{
    emit resetRobot(4);
}
