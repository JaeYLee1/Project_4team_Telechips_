#include "motorwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MotorWindow window;
    window.show();

    return app.exec();
}