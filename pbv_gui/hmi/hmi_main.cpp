#include "hmi_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    HmiWindow window;
    window.show();

    return app.exec();
}