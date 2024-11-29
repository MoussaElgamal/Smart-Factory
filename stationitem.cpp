#include "stationitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include "agent.h"
#include "dimension.h"

//
StationItem::StationItem(QColor color, QString stationType, int stationId, qreal width, qreal height)
    : color(color)
    , type(stationType)
    , id(stationId)
    , width(width)
    , height(height)
{
    setToolTip(type + QString::number(id));   // Tooltip für die Station mit ID
    setFlag(QGraphicsItem::ItemIsSelectable); // Setzt die Station als auswählbar
    setFlag(QGraphicsItem::ItemIsFocusable);  // Ermöglicht Fokus
    //setFlag(QGraphicsItem::ItemIsMovable);  // Macht das Item beweglich (optional)
}

QRectF StationItem::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void StationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Zeichne den Hintergrund der Station
    painter->setBrush(QBrush(color));
    painter->drawRect(boundingRect());

    // Setze den Text auf fett
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);

    // Zeichne den Text der Station
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, type + QString::number(id));
}

void StationItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        qDebug() << "Rechtsklick erkannt auf"
                 << type + QString::number(id); // Debugging, um den Klick zu bestätigen
        showStationInfo();                      // Zeige Station-Informationen bei Rechtsklick
    } else {
        // Weiteres Event-Handling bei anderen Maustasten
        QGraphicsItem::mousePressEvent(event);
    }
}

void StationItem::showStationInfo()
{
    QString typeOfStation;

    // Lade oder Fertigungsstation unterscheiden
    if (type == "S") {
        typeOfStation = "Fertigungsstation";
        // int state = viewInterface.getState(Agent::Type::machine, id); // Zustand der Fertigungsstation holen
    } else if (type == "L") {
        typeOfStation = "Ladestation";
        // int state = viewInterface.getState(Agent::Type::chargingPlace, id); // Zustand der Ladestation holen
    }

    int state = 0; // Beispielzustand
    QString stateString = getStateString(static_cast<Agent::State>(state));

    QMessageBox::information(nullptr,
                             "Station Info",
                             typeOfStation + " " + QString::number(id) + "\nPosition: ("
                                 + QString::number( ((x()*SCALE_FACTOR_REAL)/SCALE_FACTOR)) + ", "
                                 + QString::number( (((SCENE_HEIGHT-y())*SCALE_FACTOR_REAL)/SCALE_FACTOR)) + ")"
                                 + "\nState: " + stateString);
}

// Hilfsfunktion zur Umwandlung des Zustands in einen lesbaren deutschen String
QString StationItem::getStateString(Agent::State state)
{
    switch (state) {
    case Agent::State::error:
        return "Fehler";
    case Agent::State::free:
        return "Frei";
    case Agent::State::reserved:
        return "Reserviert";
    case Agent::State::allocated:
        return "Zugewiesen";
    default:
        return "Unbekannt";
    }
}

/*#include "stationitem.h"

StationItem::StationItem(QColor color, QString stationType, qreal width, qreal height)
    : color(color), type(stationType), width(width), height(height)
{
    // Optional: Setze die Tooltips, um die Stationen zu beschriften
    setToolTip(type);
}

// Festlegung des Bounding-Rechtecks der Station
QRectF StationItem::boundingRect() const
{
    return QRectF(0, 0, width, height);  // Rückgabe der Größe basierend auf width und height
}

// Malfunktion für die Station
void StationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Fülle die Station mit der angegebenen Farbe
    painter->setBrush(QBrush(color));
    painter->drawRect(boundingRect());  // Zeichne das Rechteck

    // Zeichne den Text (Art der Station) in der Mitte
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, type);
}

*/
/*#include "stationitem.h"

StationItem::StationItem(QColor stationColor, QString stationType) : color(stationColor), type(stationType)
{
}

QRectF StationItem::boundingRect() const
{
    return QRectF(0, 0, 50, 50);  // Rechteck, das die Station umgibt
}

void StationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(color);
    painter->drawRect(0, 0, 50, 50);  // Station als Rechteck zeichnen

    // Optionaler Text für den Stationstyp
    painter->drawText(boundingRect(), Qt::AlignCenter, type);
}
*/
