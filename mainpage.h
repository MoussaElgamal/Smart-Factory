#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QMainWindow>
#include <QDir>
#include <QButtonGroup>
#include <QGridLayout>
#include "QPushButton"
#include <QStackedLayout>
#include <QMessageBox>
#include <QLabel>

#include "sqlinterface.h"
#include "ordermanagement.h"
#include "productionoverview.h"
#include "viewinterface.h"
#include "dialog.h"

namespace Ui {
class MainPage;
}

class MainPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr,
                      SqlInterface *sqlInterface = nullptr, ViewInterface* viewinterface = nullptr);
    ~MainPage();

signals:
    void resetRobotBumper(const int &robot);
    void resetRobotPartLost(const int &robot);

private slots:
    void onClickOrdermanagement(void);
    void onClickProductionoverview(void);
    void toMainPage(void);
    void onRobotError(const int robotNumber, const QString errorText);
    void onInfo(const int &robot, const QString &infoText);
    void errorConfirmed(const int robot, const bool resetBumber, const bool resetPartLost, Dialog* dialog);

private:
    Ui::MainPage *ui;

    SqlInterface* _sqlInterface = nullptr;
    ViewInterface* _viewInterface = nullptr;

    OrderManagement* _orderManagement = nullptr;
    ProductionOverview* _productionOverview = nullptr;
    QStackedLayout* _stackedLayout = nullptr;
    QVBoxLayout* _mainLayout = nullptr;

    QString _generalInfo = "";
    QString _robot1Info = "keine Info empfangen";
    QString _robot2Info = "keine Info empfangen";
    QString _robot3Info = "keine Info empfangen";
    QString _robot4Info = "keine Info empfangen";

    QWidget* _mainPage = nullptr;
    QWidget* _mainPageBackground = nullptr;
    QWidget* _orderManagementBackground = nullptr;
    QWidget* _productionOverviewBackground = nullptr;
    QWidget* _infoline = nullptr;

    QLabel* _infotext = nullptr;

    void buildWindow(void);


};

#endif // MAINPAGE_H
