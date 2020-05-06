#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "status.h"
#include <QDebug>
#include <QString>
#include <QVector>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    /* QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, */
    /*                  &app, [url](QObject *obj, const QUrl &objUrl) { */
    /*     if (!obj && url == objUrl) */
    /*         QCoreApplication::exit(-1); */
    /* }, Qt::QueuedConnection); */
    engine.load(url);

    QVector<QString> paths;
    paths.append("m/43'/60'/1581'/0'/0"); 
    paths.append("m/44'/60'/0'/0/0");
    QString s = Status::multiAccountGenerateAndDeriveAddresses(5, 12, "", paths); 

    return app.exec();
}
