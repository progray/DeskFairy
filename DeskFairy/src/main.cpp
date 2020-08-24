#include <QtWidgets/QApplication>
#include <QWidget>
#include <QTextCodeC>
#include <QTime>
#include <locale>
#include "Live2DWidget.h"
#include "Settings.h"

int main(int argc, char *argv[])
{
    std::locale::global(std::locale(""));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    Settings::Load();

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    
    Live2DWidget* w = new Live2DWidget("Lilywhite");

    a.connect(w, &Live2DWidget::CloseSignal, &a, &QApplication::exit);

    return a.exec();
}