#include <QNetworkAccessManager>
#include <QApplication>
#include <QDebug>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    auto nam = new QNetworkAccessManager;
    auto i = nam->networkAccessible();
    return app.exec();
}