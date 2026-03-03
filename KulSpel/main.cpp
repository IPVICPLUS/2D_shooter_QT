

#include <QApplication>
#include "gameview.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameView view;

    view.setWindowTitle("simple 2D game - QT");
    view.show();


    return a.exec();
}
