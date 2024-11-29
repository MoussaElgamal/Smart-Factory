#include "robotitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include "agent.h"
#include "dimension.h"
#include <cstdlib>

//
Agent::State robotStates[4] = {Agent::State::free,
                               Agent::State::free,
                               Agent::State::free,
                               Agent::State::free};
int robotChargingStates[4] = {100, 100, 100, 100};
// Konstruktor mit Roboterfarbe und ID

RobotItem::RobotItem(QColor robotColor, int robotId, ViewInterface *viewInterface)
    : QGraphicsObject()
    , color(robotColor)
    , id(robotId)
    , blinkState(false)
{
    //batteryLevel = rand() % 101;  // Ladezustand zufällig initialisieren
    _viewInterface = viewInterface;

    connect(viewInterface, &ViewInterface::changeState, this, &RobotItem::changeState);
    connect(viewInterface, &ViewInterface::updateBatteryLevel, this, &RobotItem::updateBatteryLevel);
    _batteryLevel = 0;
    // Timer für das Blinken initialisieren
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]() {
        blinkState = !blinkState; // Blinkzustand umschalten
        update();                 // Neu zeichnen
    });
}

// Methode zur Rückgabe der Roboter-ID
int RobotItem::getId() const
{
    return id;
}

void RobotItem::changeState(Agent::Type type, int number, Agent::State state)
{
    // Bestimme den Namen der inneren Rechtecke basierend auf Typ und Nummer
    if (number != id) {
        return;
    }
    if (type == Agent::Type::robot) {
        // Überprüfe, ob die Nummer innerhalb der Grenzen des Arrays liegt
        if (number >= 0 && number < 5) {
            // Weise dem Roboter den neuen Zustand zu
            robotStates[number - 1] = state;
        }
    }
}

QColor RobotItem::stateToColor(Agent::State state)
{
    switch (state) {
    case Agent::State::error:
        return Qt::red;
    case Agent::State::free:
        return Qt::lightGray;
    case Agent::State::reserved:
        return Qt::yellow;
    case Agent::State::allocated:
        return Qt::blue;
    default:
        return Qt::lightGray;
    }
}

QRectF RobotItem::boundingRect() const
{
    return QRectF(0,
                  -15,
                  ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR),
                  ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR)); // Rechteck, das den Roboter umgibt
}


void RobotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Wenn der Ladezustand unter 35 % ist und der Roboter blinken soll
    if (_batteryLevel < 35 && blinkState) {
        painter->setBrush(Qt::red); // Warnfarbe
    } else {
        painter->setBrush(color); // Normale Farbe
    }

    // Roboterkörper zeichnen (Kreis)
    painter->drawEllipse(0,
                         -15,
                         ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR),
                         ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR));

    // Ladezustand als Text anzeigen
    QFont font = painter->font();
    font.setBold(true); // Schrift fett machen
    painter->setFont(font);

    painter->setPen(Qt::black); // Schriftfarbe schwarz

    // Zentrales Rechteck für den Ladezustandstext
    QRectF textRect(0,
                    -15,
                    ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR),
                    ((180 / SCALE_FACTOR_REAL) * SCALE_FACTOR));
    painter->drawText(textRect, Qt::AlignCenter, QString::number(_batteryLevel) + "%");
}




// Event bei einem Mausklick
void RobotItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        qDebug() << "Rechtsklick erkannt auf Robot"
                 << QString::number(id); // Debugging, um den Klick zu bestätigen
        showRobotInfo();
    }
    QGraphicsItem::mousePressEvent(event);
}

//test
int RobotItem::getBatteryState(int n)
{
    return robotChargingStates[n - 1];
}

//test
void RobotItem::updateBatteryLevel2(int number)
{
    /*
    batteryLevel = getBatteryState(number);
    //ro
    //batteryLevel += delta;
    if (batteryLevel > 100)
        batteryLevel = 100;
    if (batteryLevel < 0)
        batteryLevel = 0;

    // Blinken starten, wenn Ladezustand unter 20 % fällt
    if (batteryLevel < 20 && !blinkTimer->isActive()) {
        blinkTimer->start(300); // Blinken alle 500ms
    } else if (batteryLevel >= 20 && blinkTimer->isActive()) {
        blinkTimer->stop(); // Blinken stoppen
        blinkState = false; // Blinkzustand zurücksetzen
    }

    update(); // Neu zeichnen

*/
}

void RobotItem::updateBatteryLevel(int number, int batteryLevel)
{
    if (number != id) {
        return;
    }
    robotChargingStates[number - 1] = batteryLevel;

    _batteryLevel = getBatteryState(number);
    //ro
    //batteryLevel += delta;
    if (batteryLevel > 100)
        _batteryLevel = 100;
    if (batteryLevel < 0)
        _batteryLevel = 0;

    // Blinken starten, wenn Ladezustand unter 35 % fällt
    if (batteryLevel < 35 && !blinkTimer->isActive()) {
        blinkTimer->start(300); // Blinken alle 500ms
    } else if (batteryLevel >= 35 && blinkTimer->isActive()) {
        blinkTimer->stop(); // Blinken stoppen
        blinkState = false; // Blinkzustand zurücksetzen
    }

    update(); // Neu zeichnen
}

void RobotItem::showRobotInfo()
{
    qreal robotX = (((this->x()) * SCALE_FACTOR_REAL) / SCALE_FACTOR);
    qreal robotY = (((SCENE_HEIGHT - this->y()) * SCALE_FACTOR_REAL) / SCALE_FACTOR);
    // Abrufen des Zustands des Roboters
    //int state = viewInterface.getState(Agent::Type::robot, id);
    Agent::State state = robotStates[id - 1];
    // Zustand in lesbaren String umwandeln
    QString stateString = getStateString(static_cast<Agent::State>(state));

    // Zeige Roboterinformationen mit ID und Zustand
    QMessageBox::information(nullptr,
                             "Roboter Info",
                             "Das ist Roboter #" + QString::number(id) + "\nPosition: ("
                                 + QString::number(robotX) + ", " + QString::number(robotY) + ")"
                                 + "\nState: " + stateString
                                 + "\nLadezustand: " + QString::number(_batteryLevel) + "%");
}

// Hilfsfunktion zur Umwandlung des Zustands in einen lesbaren deutschen String
QString RobotItem::getStateString(Agent::State state)
{
    switch (state) {
    case Agent::State::error:
        return "Fehler"; // "error" zu "Fehler"
    case Agent::State::free:
        return "Frei"; // "free" zu "Frei"
    case Agent::State::reserved:
        return "Reserviert"; // "reserved" zu "Reserviert"
    case Agent::State::allocated:
        return "Zugewiesen"; // "allocated" zu "Zugewiesen"
    default:
        return "Unbekannt"; // Fallback, wenn der Zustand unbekannt ist
    }
}

RobotItem::~RobotItem()
{
    // Überprüfe, ob der Timer aktiv ist und stoppe ihn, falls ja
    if (blinkTimer->isActive()) {
        blinkTimer->stop(); // Timer anhalten
    }
    delete blinkTimer; // Timer löschen, um Speicher freizugeben
}

/* #include "robotitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPainter>

// Konstruktor
RobotItem::RobotItem(QColor robotColor) : color(robotColor)
{
}

// Bounding Rect für den Roboter
QRectF RobotItem::boundingRect() const
{
    return QRectF(0, 0, 30, 30);  // Rechteck, das den Roboter umgibt
}

// Zeichnen des Roboters
void RobotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Roboter zeichnen (z. B. ein einfacher Kreis)
    painter->setBrush(color);
    painter->drawEllipse(0, 0, 15, 15);
}

// Event bei einem Mausklick
void RobotItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        // Zeige Roboterinformationen bei Rechtsklick an
        showRobotInfo();
    }
    QGraphicsItem::mousePressEvent(event);
}

// Zeige Roboterinformationen in einer MessageBox
void RobotItem::showRobotInfo()
{
    // Aktuelle Position des Roboters
    qreal robotX = this->x() * 10;
    qreal robotY = (600 - this->y())*10;

    // Zeige eine MessageBox mit Roboterinformationen
    QMessageBox::information(nullptr, "Roboter Info",
                             "Das ist ein Roboter.\nPosition: (" + QString::number(robotX) + ", " + QString::number(robotY) + ")");
}
*/

/*#include "robotitem.h"
#include <QPainter>

int posX, posY;
RobotItem::RobotItem(QColor robotColor) : color(robotColor)
{
    // Initiale Position des Roboters (zufällig oder vorgegeben)
    //posX = rand() % 100;
    //posY = rand() % 100;
}

QRectF RobotItem::boundingRect() const
{
    return QRectF(0, 0, 30, 30);  // Rechteck, das den Roboter umgibt
}

void RobotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Roboter zeichnen (z. B. ein einfacher Kreis)
    painter->setBrush(color);
    painter->drawEllipse(0, 0, 15, 15);
}

void RobotItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        // Zeige Roboterinformationen bei Rechtsklick an
        showRobotInfo();
    }
    QGraphicsItem::mousePressEvent(event);
}

void RobotItem::showRobotInfo()
{
    // Zeige eine MessageBox mit Roboterinformationen
    QMessageBox::information(nullptr, "Roboter Info", "Das ist ein Roboter.\nPosition: (" + QString::number(posX) + ", " + QString::number(posY) + ")");
}
*/
