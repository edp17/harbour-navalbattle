#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>
#include <QPoint>

class GameEngine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList playerCells READ playerCells NOTIFY boardsChanged)
    Q_PROPERTY(int elapsedSeconds READ elapsedSeconds NOTIFY elapsedSecondsChanged)
    Q_PROPERTY(int playerShots READ playerShots NOTIFY playerShotsChanged)
    Q_PROPERTY(QVariantList enemyCells READ enemyCells NOTIFY boardsChanged)
    Q_PROPERTY(QVariantList playerShipKinds READ playerShipKinds NOTIFY boardsChanged)
    Q_PROPERTY(QVariantList enemyShipKinds READ enemyShipKinds NOTIFY boardsChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString lastAction READ lastAction NOTIFY statusChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY statusChanged)
    Q_PROPERTY(bool playerWon READ playerWon NOTIFY statusChanged)
    Q_PROPERTY(bool revealEnemyFleet READ revealEnemyFleet WRITE setRevealEnemyFleet NOTIFY revealEnemyFleetChanged)

    // Manual fleet placement (setup mode)
    Q_PROPERTY(bool setupMode READ setupMode NOTIFY setupModeChanged)
Q_PROPERTY(bool noTouchRule READ noTouchRule WRITE setNoTouchRule NOTIFY setupChanged)
    Q_PROPERTY(QVariantList setupPlaced READ setupPlaced NOTIFY setupChanged)
    Q_PROPERTY(int setupSelectedIndex READ setupSelectedIndex WRITE setSetupSelectedIndex NOTIFY setupChanged)
    Q_PROPERTY(bool setupHorizontal READ setupHorizontal WRITE setSetupHorizontal NOTIFY setupChanged)
    Q_PROPERTY(int setupHistoryCount READ setupHistoryCount NOTIFY setupChanged)
    Q_PROPERTY(bool playerTurn READ playerTurn NOTIFY statusChanged)
    Q_PROPERTY(int lastPlayerShotX READ lastPlayerShotX NOTIFY statusChanged)
    Q_PROPERTY(int lastPlayerShotY READ lastPlayerShotY NOTIFY statusChanged)
    Q_PROPERTY(int lastAiShotX READ lastAiShotX NOTIFY statusChanged)
    Q_PROPERTY(int lastAiShotY READ lastAiShotY NOTIFY statusChanged)
    Q_PROPERTY(int aiDelayMs READ aiDelayMs WRITE setAiDelayMs NOTIFY statusChanged)
Q_PROPERTY(int aiDifficulty READ aiDifficulty WRITE setAiDifficulty NOTIFY statusChanged)

public:
    explicit GameEngine(QObject *parent = nullptr);

    enum Cell : int {
        Unknown = 0,
        Water = 1,
        Ship = 2,
        Miss = 3,
        Hit = 4,
        Sunk = 5
    };
    Q_ENUM(Cell)

    enum ShipKind : int {
        NoShip = 0,
        Destroyer = 1,
        Submarine = 2,
        Cruiser = 3,
        Battleship = 4,
        Carrier = 5
    };
    Q_ENUM(ShipKind)

    Q_INVOKABLE void newGame();
    Q_INVOKABLE bool placeSetupShip(int shipIndex, int x, int y);
    Q_INVOKABLE void rotateSetupShip();
    Q_INVOKABLE bool undoSetupShip();
    Q_INVOKABLE void rebuildSetupHistory();
    Q_INVOKABLE void startBattle();
    Q_INVOKABLE void autoPlacePlayerFleet();
    Q_INVOKABLE void reshufflePlayerShips();
    Q_INVOKABLE void playerFire(int x, int y);

    Q_INVOKABLE bool saveToDisk();
    Q_INVOKABLE bool loadFromDisk();
    Q_INVOKABLE bool clearSavedGame();

    QVariantList playerCells() const;
    QVariantList enemyCells() const;

    QVariantList playerShipKinds() const;
    QVariantList enemyShipKinds() const;

    QString statusText() const { return m_statusText; }
    QString lastAction() const { return m_lastAction; }
    bool gameOver() const { return m_gameOver; }
        int playerShots() const { return m_playerShots; }
int elapsedSeconds() const { return m_elapsedSeconds; }
    bool playerWon() const { return m_playerWon; }
    bool revealEnemyFleet() const { return m_revealEnemyFleet; }
    bool playerTurn() const { return m_playerTurn; }
    int lastPlayerShotX() const { return m_lastPlayerShotX; }
    int lastPlayerShotY() const { return m_lastPlayerShotY; }
    int lastAiShotX() const { return m_lastAiShotX; }
    int lastAiShotY() const { return m_lastAiShotY; }
    int aiDelayMs() const { return m_aiDelayMs; }
    void setAiDelayMs(int ms) { m_aiDelayMs = (ms < 0 ? 0 : ms); emit statusChanged(); }
int aiDifficulty() const { return m_aiDifficulty; }
    bool setupMode() const { return m_setupMode; }
    bool noTouchRule() const;
    void setNoTouchRule(bool v);
    QVariantList setupPlaced() const;
    int setupSelectedIndex() const { return m_setupSelectedIndex; }
    void setSetupSelectedIndex(int idx);
    bool setupHorizontal() const { return m_setupHorizontal; }
    int setupHistoryCount() const { return m_setupHistory.size(); }
    void setSetupHorizontal(bool h);
void setAiDifficulty(int d) { if (d < 0) d = 0; if (d > 2) d = 2; m_aiDifficulty = d; aiReset(); emit statusChanged(); }

    // Debug helper (goes to debug.log via installed message handler)
    Q_INVOKABLE void debug(const QString &msg);

public slots:
    void setRevealEnemyFleet(bool on);

signals:
    void setupModeChanged();
    void setupChanged();
    void playerWonChanged();
    void gameOverChanged();
        void playerShotsChanged();
void elapsedSecondsChanged();
    void revealEnemyFleetChanged();
    void boardsChanged();
    void statusChanged();
    void gameFinished(bool playerWon, int elapsedSeconds);

private:
    void initSetupDefaults();

    // Timer
    QElapsedTimer m_elapsedTimer;
    QTimer m_elapsedTick;
        int m_playerShots = 0;
int m_elapsedSeconds = 0;
    int m_elapsedSecondsBase = 0;
    static constexpr int N = 12;
    static int idx(int x, int y) { return y * N + x; }
    bool inBounds(int x, int y) const { return x >= 0 && x < N && y >= 0 && y < N; }

    struct ShipInfo {
        int length = 0;
        QVector<int> cells;
        int hits = 0;
        QString name;
        bool isSunk() const { return hits >= length; }
    };

    struct Board {
        QVector<int> cells;
        QVector<ShipInfo> ships;
    };

    void resetBoards();
    void placeFleetRandom(Board &b);
    bool tryPlaceShip(Board &b, int length, const QString &name, bool noTouchRule);
    bool canPlaceAt(const Board &b, int x, int y, int length, bool horizontal, bool noTouchRule) const;
    void applyShip(Board &b, int x, int y, int length, bool horizontal, const QString &name);

    bool resolveShot(Board &targetBoard, int x, int y, bool attackerIsPlayer, QString *outMessage, int *outSunkLen = nullptr);
    ShipInfo* shipContaining(Board &b, int cellIndex);

    void aiTakeTurn();
    void aiReset();
    QPoint aiChooseShot();
    void aiNotifyResult(const QPoint &p, bool hit, bool sunk, int sunkLen);

    void setStatus(const QString &status, const QString &action);
    void emitAllChanged();
    QString saveFilePath() const;
    QVariantMap toVariantMap() const;
    bool fromVariantMap(const QVariantMap &m);
    int randomBounded(int upper) const;
    void scheduleSave();

private:
    Board m_player;
    Board m_enemy;

    QVector<bool> m_enemyRevealed;

    bool m_gameOver = false;
    bool m_playerWon = false;
    bool m_revealEnemyFleet = false;
    bool m_playerTurn = true;
    QString m_statusText;
    QString m_lastAction;

    int m_lastPlayerShotX = -1;
    int m_lastPlayerShotY = -1;
    int m_lastAiShotX = -1;
    int m_lastAiShotY = -1;
    int m_aiDelayMs = 350;
    int m_aiDifficulty = 1; // 0 Easy, 1 Medium, 2 Hard

    // Manual fleet placement (setup mode)
    bool m_setupMode = false;
    int m_setupSelectedIndex = 0;
    bool m_setupHorizontal = true;
    QVector<int> m_setupLens;
    QVector<QString> m_setupNames;
    QVector<bool> m_setupPlaced;

    struct SetupPlacement {
        int shipIndex = -1;
        int x = 0;
        int y = 0;
        bool horizontal = true;
        int len = 0;
        QString name;
    };
    QVector<SetupPlacement> m_setupHistory;

    bool m_noTouchRule = false;

    QVector<bool> m_aiTried;
    QVector<QPoint> m_aiTargets;
    QVector<QPoint> m_aiHits; // hits on current unsunk ship (Hard)
    QVector<int> m_aiRemainingLens;
    bool m_booting = true;
};
