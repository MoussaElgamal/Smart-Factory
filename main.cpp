#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <factory.h>
#include "simulationwindow.h"
#include "dialog.h"
// #include "orderwindow.h"
// #include "mainpage.h"
// #include "sqlinterface.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Produktionsanlage_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    Factory factory;

    MainPage* mp = factory.createProductionPlant();;
    mp->showFullScreen();



    /*
    MainWindow mw;
    mw.show();
    */
    return a.exec();
}

