#include "hmi_window.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);

    HmiWindow window;
    window.resize(1280, 720);
    window.show();
    window.raise();
    window.activateWindow();

    qDebug() << "[HMI] window visible =" << window.isVisible();

    return app.exec();
}