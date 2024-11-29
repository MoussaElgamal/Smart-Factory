#include "mainpage.h"
#include "ui_mainpage.h"

MainPage::MainPage(QWidget *parent,
                   SqlInterface *sqlInterface, ViewInterface* viewInterface)
    : QMainWindow(parent)
    , ui(new Ui::MainPage)
{
    _sqlInterface = sqlInterface;
    _viewInterface = viewInterface;
    ui->setupUi(this);

    connect(_viewInterface, &ViewInterface::sendRobotError, this, &MainPage::onRobotError);
    connect(_viewInterface, &ViewInterface::sendInfo, this, &MainPage::onInfo);
    connect(this, &MainPage::resetRobotBumper, _viewInterface, &ViewInterface::onResetRobotBumper);
    connect(this, &MainPage::resetRobotPartLost, _viewInterface, &ViewInterface::onResetRobotPartLost);

    buildWindow();
}



MainPage::~MainPage()
{
    delete ui;
}

void MainPage::onClickOrdermanagement()
{
    _stackedLayout->setCurrentWidget(_orderManagementBackground);
    _stackedLayout->setCurrentWidget(_orderManagement);
}

void MainPage::onClickProductionoverview()
{
    _stackedLayout->setCurrentWidget(_productionOverviewBackground);
    _stackedLayout->setCurrentWidget(_productionOverview);
    //this->showMaximized();
}

void MainPage::toMainPage()
{
    _stackedLayout->setCurrentWidget(_mainPageBackground);
    _stackedLayout->setCurrentWidget(_mainPage);
    //this->showMaximized();
}

void MainPage::onRobotError(const int robotNumber, const QString errorText)
{
    /*
    int ret = QMessageBox::warning(this, QString("Fehler Roboter %1").arg(robotNumber), errorText, QMessageBox::StandardButton::Reset, QMessageBox::Button::Close);
    if (ret == QMessageBox::Reset && errorText.contains("Bumper")) {
        emit resetRobotBumper(robotNumber);
    }
    if (ret == QMessageBox::Reset && errorText.contains("Bauteil")) {
        emit resetRobotPartLost(robotNumber);
    }
    */
    Dialog* dialog = new Dialog(QString("Fehler Roboter %1").arg(robotNumber), errorText, robotNumber, errorText.contains("Bumper"), errorText.contains("Bauteil"));
    QObject::connect(dialog, &Dialog::onReset, this, &MainPage::errorConfirmed);
}

void MainPage::onInfo(const int &robot, const QString &infoText)
{
    if (robot == 0) {
        _generalInfo = infoText;
    }
    if (robot == 1) {
        _robot1Info = infoText;
    }
    if (robot == 2) {
        _robot2Info = infoText;
    }
    if (robot == 3) {
        _robot3Info = infoText;
    }
    if (robot == 4) {
        _robot4Info = infoText;
    }
    QString info = "Allgemein: ";
    info.append(_generalInfo);
    info.append(";\nRoboter 1: ");
    info.append(_robot1Info);
    info.append("; Roboter 2: ");
    info.append(_robot2Info);
    info.append(";\nRoboter 3: ");
    info.append(_robot3Info);
    info.append("; Roboter 4: ");
    info.append(_robot4Info);
    _infotext->setText(info);
}

void MainPage::errorConfirmed(const int robot, const bool resetBumper, const bool resetPartLost, Dialog* dialog)
{
    if (resetBumper) {
        emit resetRobotBumper(robot);
    }
    if (resetPartLost) {
        emit resetRobotPartLost(robot);
    }
    QObject::disconnect(dialog, &Dialog::onReset, this, &MainPage::errorConfirmed);
}

void MainPage::buildWindow()
{
    setWindowTitle("Smart Factory");

    QWidget* mainWidget = new QWidget;
    setCentralWidget(mainWidget);
    _mainLayout = new QVBoxLayout(mainWidget);
    _stackedLayout = new QStackedLayout(_mainLayout); //create new stacked layout and connect to mainWidget
    _stackedLayout->setStackingMode(QStackedLayout::StackAll);

    _orderManagement = new OrderManagement(nullptr, _sqlInterface, _viewInterface);
    connect(_orderManagement, &OrderManagement::backToMainPage, this, &MainPage::toMainPage);
    _orderManagementBackground = new QWidget;
    _orderManagementBackground->setStyleSheet("border-image: url(../../AuftragsmanagementHintergrund.png) 0 0 0 0 stretch stretch;");

    _productionOverview = new ProductionOverview(nullptr, nullptr, nullptr, _viewInterface);
    connect(_productionOverview, &ProductionOverview::backToMainPage, this, &MainPage::toMainPage);
    _productionOverviewBackground = new QWidget;
    _productionOverviewBackground->setStyleSheet("border-image: url(../../AuftragsmanagementHintergrund.png) 0 0 0 0 stretch stretch;");

    QSize iconSize(503,117);

    _mainPage = new QWidget();
    QPushButton* buttonOrdermanagement = new QPushButton();
    QIcon iconOrdermanagement = (QPixmap("../../Auftragsmanagement.png"));
    buttonOrdermanagement->setIcon(iconOrdermanagement);
    buttonOrdermanagement->setIconSize(iconSize);
    buttonOrdermanagement->setFixedSize(iconSize);
    connect(buttonOrdermanagement, &QPushButton::clicked, this, &MainPage::onClickOrdermanagement);
    QPushButton* buttonProductionoverview = new QPushButton();
    QIcon iconProductionoverview = (QPixmap("../../Produktionsübersicht.png"));
    buttonProductionoverview->setIcon(iconProductionoverview);
    buttonProductionoverview->setIconSize(iconSize);
    buttonProductionoverview->setFixedSize(iconSize);
    connect(buttonProductionoverview, &QPushButton::clicked, this, &MainPage::onClickProductionoverview);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(buttonOrdermanagement);
    layout->addWidget(buttonProductionoverview);
    layout->setAlignment(Qt::AlignCenter);
    _mainPage->setLayout(layout);
    //Hintergrundbild einstellen
    _mainPageBackground = new QWidget;
    _mainPageBackground->setStyleSheet("border-image: url(../../Startscreen.png) 0 0 0 0 stretch stretch;");

    //Infoline
    _infoline = new QWidget();
    _infoline->setFixedHeight(0.15*mainWidget->height());
    _infoline->setFixedWidth(mainWidget->width());
    _infoline->setStyleSheet("border-image: url(../../Produktionsübersicht.png) 0 0 0 0 stretch stretch;");
    _infotext = new QLabel(_infoline);
    _infotext->resize(mainWidget->width(), 0.15*mainWidget->height());
    _infotext->setTextFormat(Qt::PlainText);
    //_infotext->setAlignment(Qt::AlignLeft);
    _infotext->setStyleSheet("QLabel{font-size: 12pt;}");
    _infotext->setText("keine Meldung empfangen");
    _mainLayout->addWidget(_infoline);

    _stackedLayout->addWidget(_mainPageBackground);
    _stackedLayout->addWidget(_mainPage);
    _stackedLayout->addWidget(_orderManagementBackground);
    _stackedLayout->addWidget(_orderManagement);

    _stackedLayout->addWidget(_productionOverviewBackground);
    _stackedLayout->addWidget(_productionOverview);

    _stackedLayout->setCurrentWidget(_mainPageBackground);
    _stackedLayout->setCurrentWidget(_mainPage);
}
