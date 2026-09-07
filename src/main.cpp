#include <sailfishapp.h>

#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml>
#include <QQmlContext>
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

#include <QDir>

#include "GameEngine.h"
#include "SettingsManager.h"

#ifndef NAVALBATTLE_VERSION
#define NAVALBATTLE_VERSION "development"
#endif

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    // Stable identifiers for storage paths
    QCoreApplication::setOrganizationName(QStringLiteral("harbour"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-navalbattle"));
    QCoreApplication::setApplicationVersion(QStringLiteral(NAVALBATTLE_VERSION));

    QTranslator translator;
    const QString translationDirectory = SailfishApp::pathTo(
        QStringLiteral("translations")).toLocalFile();
    if (translator.load(QLocale(), QStringLiteral("harbour-navalbattle"),
                        QStringLiteral("-"), translationDirectory)) {
        app->installTranslator(&translator);
    }

    QQuickView *view = SailfishApp::createView();
    GameEngine *engine = new GameEngine(view);
    SettingsManager *settings = new SettingsManager(view);
    engine->setApplicationActive(app->applicationState() == Qt::ApplicationActive);
    view->rootContext()->setContextProperty(QStringLiteral("settings"), settings);
    view->rootContext()->setContextProperty(QStringLiteral("engine"), engine);
    view->rootContext()->setContextProperty(
        QStringLiteral("appVersion"), QCoreApplication::applicationVersion());

    QObject::connect(engine, &GameEngine::gameFinished, settings, [settings, engine](bool playerWon, int elapsedSeconds) {
        if (!playerWon) return;
        if (elapsedSeconds <= 0) return;
        QString name = settings->playerName().trimmed();
        if (name.isEmpty()) name = QStringLiteral("Player");
        settings->addBestTime(name, elapsedSeconds, engine->playerShots());
    });
    // Restore previous state; if none, start a fresh game.
    if (!engine->loadFromDisk()) {
        engine->newGame();
    }

    QObject::connect(app, &QGuiApplication::applicationStateChanged, engine, [engine](Qt::ApplicationState state) {
        engine->setApplicationActive(state == Qt::ApplicationActive);
        if (state == Qt::ApplicationInactive || state == Qt::ApplicationSuspended) {
            engine->saveToDisk();
        }
    });
    QObject::connect(app, &QCoreApplication::aboutToQuit, engine, [engine]() {
        engine->saveToDisk();
    });

    view->setSource(SailfishApp::pathTo("qml/harbour-navalbattle.qml"));
    view->show();

    return app->exec();
}
