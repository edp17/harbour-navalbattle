#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QVariantList>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showCoordinates READ showCoordinates WRITE setShowCoordinates NOTIFY changed)
    Q_PROPERTY(int aiDelayMs READ aiDelayMs WRITE setAiDelayMs NOTIFY changed)
    Q_PROPERTY(int cellSizePx READ cellSizePx WRITE setCellSizePx NOTIFY changed)
    Q_PROPERTY(int hitMissHoldMs READ hitMissHoldMs WRITE setHitMissHoldMs NOTIFY changed)
    Q_PROPERTY(int thinkingHoldMs READ thinkingHoldMs WRITE setThinkingHoldMs NOTIFY changed)
Q_PROPERTY(int aiDifficulty READ aiDifficulty WRITE setAiDifficulty NOTIFY changed)
    Q_PROPERTY(int aiPace READ aiPace WRITE setAiPace NOTIFY changed)
    Q_PROPERTY(bool noTouchRule READ noTouchRule WRITE setNoTouchRule NOTIFY changed)
    Q_PROPERTY(bool showThinkingPopup READ showThinkingPopup WRITE setShowThinkingPopup NOTIFY changed)
    Q_PROPERTY(bool showGameOverOverlay READ showGameOverOverlay WRITE setShowGameOverOverlay NOTIFY changed)
    Q_PROPERTY(bool showSunkShipSprites READ showSunkShipSprites WRITE setShowSunkShipSprites NOTIFY changed)
    Q_PROPERTY(bool tintEnemyShips READ tintEnemyShips WRITE setTintEnemyShips NOTIFY changed)
    Q_PROPERTY(QString playerName READ playerName WRITE setPlayerName NOTIFY changed)
    Q_PROPERTY(QVariantList bestTimes READ bestTimes NOTIFY changed)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    bool showCoordinates() const;
    void setShowCoordinates(bool v);

    int aiPace() const;
    void setAiPace(int v);

    int aiDelayMs() const;
    int cellSizePx() const;

    int hitMissHoldMs() const;
    int thinkingHoldMs() const;
int aiDifficulty() const;

    bool noTouchRule() const;
    void setNoTouchRule(bool v);
    bool showThinkingPopup() const;
    void setShowThinkingPopup(bool v);
    bool showGameOverOverlay() const;
    void setShowGameOverOverlay(bool v);
    bool showSunkShipSprites() const;
    void setShowSunkShipSprites(bool v);
    bool tintEnemyShips() const;
    void setTintEnemyShips(bool v);
    QString playerName() const;
    QVariantList bestTimes();

    void setHitMissHoldMs(int ms);
    void setThinkingHoldMs(int ms);
void setAiDifficulty(int d);
    void setPlayerName(const QString &name);

    void setAiDelayMs(int ms);
    Q_INVOKABLE void setCellSizePx(int px);
    Q_INVOKABLE void addBestTime(int elapsedSeconds);
    Q_INVOKABLE void addBestTime(const QString &playerName, int elapsedSeconds, int shots);
    void addBestTime(const QString &playerName, int elapsedSeconds);
    Q_INVOKABLE void clearBestTimes();

signals:
    void changed();

private:
    QSettings m_settings;
};
