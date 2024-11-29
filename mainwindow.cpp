#include "mainwindow.h"
#include "qdatetime.h"
#include "ui_mainwindow.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <sqlinterface.h>


//SqlInterface SqlInterface;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // QTimer initialisieren
    timer = new QTimer(this);

    // Verbinde den Timer-Signal mit der Funktion, die regelmäßig aufgerufen werden soll
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshData);

    // Timer-Intervall auf 500ms setzen und starten
    timer->start(500);

}

MainWindow::~MainWindow()
{
    emit backToMainPage();
    delete ui;
}

/*!
 * \brief MainWindow::on_refreshData
 * This function is triggered when the "Read Data" button is clicked. It retrieves data from the database to populate two tables in the UI
 */
void MainWindow::refreshData()
{
    // Mapping von StatusNrFK zu Statusnamen und Farben
    QMap<int, QString> statusMapping;
    statusMapping[1] = "Wartend";
    statusMapping[2] = "In Bearbeitung";
    statusMapping[3] = "Abgeschlossen";

    QMap<int, QString> artikelMapping;
    artikelMapping[1] = "A";
    artikelMapping[2] = "B";
    artikelMapping[3] = "C";
    artikelMapping[4] = "D";

    // Farben für StatusNrFK mit Transparenz festlegen
    QMap<int, QColor> statusColorMapping;
    statusColorMapping[1] = QColor(255, 0, 0, 150);    // Rot mit Transparenz
    statusColorMapping[2] = QColor(255, 165, 0, 150);  // Orange mit Transparenz
    statusColorMapping[3] = QColor(0, 255, 0, 150);    // Grün mit Transparenz

    // Befüllen der ersten Tabelle Inhalte aus t02 und t01 über einen JOIN
    QSqlQuery Query_get_Data(db);
    Query_get_Data.prepare(
        "SELECT t02bestellung.BestellNrPK, t01kunden.Name, t02bestellung.StatusNrFK, t02bestellung.Zeitstempel "
        "FROM t02bestellung "
        "INNER JOIN t01kunden ON t02bestellung.KundenNrFK = t01kunden.KundenNrPK "
        "ORDER BY t02bestellung.BestellNrPK DESC"
        );

    // Führe die Abfrage aus und prüfe auf Fehler
    if (!Query_get_Data.exec()) {
        qDebug() << "Error executing query:" << Query_get_Data.lastError().text();
        return;
    }

    int RowNumber = 0;
    ui->tableWidget_tbl02->setRowCount(Query_get_Data.size());


    while (Query_get_Data.next()) {

        ui->tableWidget_tbl02->setItem(RowNumber, 0, new QTableWidgetItem(QString::number(Query_get_Data.value("BestellNrPK").toInt())));

        // Den Namen aus der Tabelle t01kunden anzeigen
        QString kundenName = Query_get_Data.value("Name").toString();
        ui->tableWidget_tbl02->setItem(RowNumber, 1, new QTableWidgetItem(kundenName));

        int statusNrFK = Query_get_Data.value("StatusNrFK").toInt();
        QString statusName = statusMapping.value(statusNrFK, "Unbekannt");
        ui->tableWidget_tbl02->setItem(RowNumber, 3, new QTableWidgetItem(statusName));

        QDateTime timestamp = Query_get_Data.value("Zeitstempel").toDateTime();
        ui->tableWidget_tbl02->setItem(RowNumber, 2, new QTableWidgetItem(timestamp.toString("dd.MM.yyyy HH:mm:ss")));

        // Setze die Hintergrundfarbe auf Basis des Status
        QColor rowColor = statusColorMapping.value(statusNrFK, QColor(255, 255, 255, 150));  // Default Farbe weiß
        for (int col = 0; col < ui->tableWidget_tbl02->columnCount(); ++col) {
            ui->tableWidget_tbl02->item(RowNumber, col)->setBackground(rowColor);
        }

        RowNumber++;
    }

    // Befüllen der zweiten Tabelle Inhalte aus t05
    QSqlQuery Query_get_Data_2(db);
    Query_get_Data_2.prepare("SELECT * FROM t05bestellposition order by BestellpositionNrPK desc");
    if (Query_get_Data_2.exec())
    {
        int RowNumber_2 = 0;
        ui->tableWidget_tbl05->setRowCount(Query_get_Data_2.size());
        while (Query_get_Data_2.next())
        {
            ui->tableWidget_tbl05->setItem(RowNumber_2, 0, new QTableWidgetItem(QString::number(Query_get_Data_2.value("BestellNrFK").toInt())));
            ui->tableWidget_tbl05->setItem(RowNumber_2, 1, new QTableWidgetItem(QString::number(Query_get_Data_2.value("BestellpositionNrPK").toInt())));

            int artikelNrFK = Query_get_Data_2.value("ArtikelNrFK").toInt();
            QString artikelName = artikelMapping.value(artikelNrFK, "Unbekannt");
            ui->tableWidget_tbl05->setItem(RowNumber_2, 2, new QTableWidgetItem(artikelName));

            int statusNrFK = Query_get_Data_2.value("StatusNrFK").toInt();
            QString statusName = statusMapping.value(statusNrFK, "Unbekannt");
            ui->tableWidget_tbl05->setItem(RowNumber_2, 3, new QTableWidgetItem(statusName));

            // Setze die Hintergrundfarbe auf Basis des Status
            QColor rowColor = statusColorMapping.value(statusNrFK, QColor(255, 255, 255, 150));  // Default Farbe weiß
            for (int col = 0; col < ui->tableWidget_tbl05->columnCount(); ++col) {
                ui->tableWidget_tbl05->item(RowNumber_2, col)->setBackground(rowColor);
            }

            RowNumber_2++;
        }
    }

}



/*!
 * \brief MainWindow::on_pushButton_InsertNew_clicked
 * This function is triggered when the "Place Order" button is clicked. It checks the values in four product quantity spin boxes (Product A, B, C, D).
 * If at least one of the spin boxes has a quantity greater than zero, the function proceeds to create a new order.
 */
void MainWindow::on_pushButton_InsertNew_clicked()
{
    // Überprüfen, ob mindestens eine der SpinBoxen eine Anzahl > 0 hat
    bool isAnySpinBoxNonZero = false;

    // Annahme: Die SpinBoxen sind in der UI als spinBox_AnzA, spinBox_AnzB, spinBox_AnzC, spinBox_AnzD definiert
    QSpinBox *spinBoxes[] = {ui->spinBox_AnzA, ui->spinBox_AnzB, ui->spinBox_AnzC, ui->spinBox_AnzD};

    QList<int> parts;
    QList<Part::ProductType> productList;

    // KundenNr basierend auf dem ausgewählten Radio Button setzen
    int customerID = 0;  // Default-Wert
    if (ui->radioButton_Gewerk1->isChecked()) {
        customerID = 1;  // KundenNr für "Gewerk 1"
    } else if (ui->radioButton_Professoren->isChecked()) {
        customerID = 2;  // KundenNr für "Professoren"
    }

    // Überprüfen, ob ein Team ausgewählt wurde
    if (customerID == 0) {
        qDebug() << "Kein Team ausgewählt. Bestellung wird nicht angelegt.";
        return;
    }

    int index = 0;
    for (auto spinBox : spinBoxes){
        int value = spinBox->value();  // Den aktuellen Wert der SpinBox auslesen
        if (value > 0) {
            parts.append(value);  // Anzahl hinzufügen
            productList.append(static_cast<Part::ProductType>(index + 1));  // Produkttyp hinzufügen
            isAnySpinBoxNonZero = true;
        }
        index++; // Index für den Produkttyp erhöhen
    }

    // Wenn alle SpinBoxen 0 sind, Funktion nicht ausführen
    if (!isAnySpinBoxNonZero) {
        qDebug() << "Keine Artikel ausgewählt. Bestellung wird nicht angelegt.";
        return;
    }

    // Bestellung in die Datenbank senden

    // Anzeige aktualisieren
    refreshData();

    // Setze alle SpinBoxen auf 0 zurück
    for (auto spinBox : spinBoxes) {
        spinBox->setValue(0);  // Setzt den Wert der SpinBox auf 0 zurück
    }

    // Close the database connection
   // SqlInterface.closeDatabaseConnection();
}


/*!
 * \brief MainWindow::on_spinBox_logID_valueChanged
 * \param werkstueckID The ID of the workpiece whose production log is being retrieved.
 *
 * This function is triggered when the value of the spin box (representing the workpiece ID) changes.
 * It retrieves the production log entries for the specified workpiece (werkstueckID) from the database
 * and displays them in the log table widget.
 */
void MainWindow::on_spinBox_logID_valueChanged(int werkstueckID)
{
    // Mapping von MaschinenNummer zu Orten
    QMap<int, QString> ortMapping;
    ortMapping[1] = "Rohteillager 1";
    ortMapping[2] = "Rohteillager 2";
    ortMapping[3] = "Maschine 1";
    ortMapping[4] = "Maschine 2";
    ortMapping[5] = "Maschine 3";
    ortMapping[6] = "Maschine 4";
    ortMapping[7] = "Fertigteillager 1";
    ortMapping[8] = "Fertigteillager 2";

    // Connect to the database
    //SqlInterface.connectToDatabase();

    // Clear the tableWidget_log before populating it
    ui->tableWidget_log->clearContents();
    ui->tableWidget_log->setRowCount(0);  // Reset the row count

    // SQL query to fetch log data for the given WerkstückID
    QSqlQuery queryLog(db);
    queryLog.prepare("SELECT BestellpositionNrFK, MaschinenNr, Zeitstempel FROM t07log WHERE BestellpositionNrFK = :werkstueckID");
    queryLog.bindValue(":werkstueckID", werkstueckID);

    // Execute the query and populate the table
    if (queryLog.exec()) {
        int row = 0;

        // Set column headers
        ui->tableWidget_log->setColumnCount(3);
        ui->tableWidget_log->setHorizontalHeaderLabels(QStringList() << "WerkstückID" << "Ort" << "Zeitstempel");

        while (queryLog.next()) {
            // Add a new row for each log entry
            ui->tableWidget_log->insertRow(row);

            // Fetch data from the query
            int werkstueckID = queryLog.value("BestellpositionNrFK").toInt();
            int maschine = queryLog.value("MaschinenNr").toInt();
            QDateTime timestamp = queryLog.value("Zeitstempel").toDateTime();  // Retrieve the datetime value
            QString formattedTimestamp = timestamp.toString("dd.MM.yyyy HH:mm:ss");  // Format the timestamp

            // Map the machine number to a location name
            QString ortName = ortMapping.value(maschine, "Unbekannter Ort");

            // Populate the row with data
            ui->tableWidget_log->setItem(row, 0, new QTableWidgetItem(QString::number(werkstueckID)));
            ui->tableWidget_log->setItem(row, 1, new QTableWidgetItem(ortName));  // Use mapped location name
            ui->tableWidget_log->setItem(row, 2, new QTableWidgetItem(formattedTimestamp));  // Use formatted timestamp

            row++;
        }

        if (row == 0) {
            qDebug() << "No entries found for WerkstückID " << werkstueckID;
        }
    } else {
        qDebug() << "Error executing query: " << queryLog.lastError().text();
    }

    // Close the database connection
    //SqlInterface.closeDatabaseConnection();
}

