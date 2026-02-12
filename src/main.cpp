#include <sailfishapp.h>

#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml>
#include <QQmlContext>
#include <QCoreApplication>

#include <QDir>

#include "GameEngine.h"
#include "SettingsStore.h"
#include "SettingsManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    // Stable identifiers for storage paths
    QCoreApplication::setOrganizationName(QStringLiteral("harbour"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-navalbattle"));

    QQuickView *view = SailfishApp::createView();
    GameEngine *engine = new GameEngine(view);
    SettingsManager *settings = new SettingsManager(view);
    view->rootContext()->setContextProperty(QStringLiteral("settings"), settings);
    view->rootContext()->setContextProperty(QStringLiteral("engine"), engine);

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
