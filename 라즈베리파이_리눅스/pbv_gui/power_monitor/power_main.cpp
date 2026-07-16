#include "power_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PowerWindow window;
    window.show();

    return app.exec();
}