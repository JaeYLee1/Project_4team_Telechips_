#include "temperaturewindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TemperatureWindow window;
    window.show();

    return app.exec();
}