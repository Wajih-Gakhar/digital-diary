#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Load custom fonts from resources
    int fontId1 = QFontDatabase::addApplicationFont(":/fonts/ComicNeue-Bold.ttf");
    int fontId2 = QFontDatabase::addApplicationFont(":/fonts/Quicksand-Regular.ttf");

    // Optional: Set as default application font
    if (fontId1 != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId1);
        if (!fontFamilies.isEmpty()) {
            QString family = fontFamilies.at(0);
            QFont font(family, 10);
            a.setFont(font);
            qDebug() << "Loaded font:" << family;
        }
    }

    // Load QSS from resources
    QFile f(":/modern_diary.qss");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&f);
        QString styleSheet = ts.readAll();
        a.setStyleSheet(styleSheet);
        f.close();
        qDebug() << "Stylesheet loaded successfully";
    }
    else {
        qDebug() << "Failed to load stylesheet";
    }

    MainWindow w;
    w.show();

    return a.exec();
}