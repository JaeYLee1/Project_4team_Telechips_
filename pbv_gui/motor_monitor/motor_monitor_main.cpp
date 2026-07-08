#include "motor_monitor_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MotorMonitorWindow window;
    window.show();

    return app.exec();
}