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
    qmlRegisterSingletonType<Status>("im.status.desktop.Status", 1, 0, "Status",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *{
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)

            Status * status = new Status();
            return status;
        });


    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    engine.load(url);
    //QString s = Status::multiAccountGenerateAndDeriveAddresses(5, 12, ""); 

    return app.exec();
}
