#include "GameEngine.h"

// Needed for placement new used by Qt containers on some toolchains.
#include <new>
#include <QtGlobal>
#include <QDateTime>
#include <algorithm>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDebug>
#include <QTimer>

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_enemyRevealed(N*N, false)
    , m_aiTried(N*N, false)
{
    initSetupDefaults();

    m_elapsedTick.setInterval(1000);
    m_elapsedTick.setSingleShot(false);
    connect(&m_elapsedTick, &QTimer::timeout,
            this, &GameEngine::updateElapsedSeconds);

    // Ranked time counts only the periods in which the player can act.
    m_elapsedMillisecondsBase = 0;
    m_elapsedSeconds = 0;
    m_elapsedTick.stop();

    seedRandom(static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0xffffffff));
    m_booting = true;
    // Do NOT start a new game here; main.cpp decides whether to restore or start fresh.
}

void GameEngine::debug(const QString &msg)
{
    Q_UNUSED(msg)
}

bool GameEngine::noTouchRule() const
{
    return m_noTouchRule;
}

void GameEngine::setNoTouchRule(bool v)
{
    if (m_noTouchRule == v) return;
    m_noTouchRule = v;

    // A fleet placed while touching was allowed may become illegal when the
    // stricter rule is enabled. Preserve valid layouts; clear only invalid ones.
    if (m_setupMode && v && !fleetSatisfiesNoTouchRule(m_player)) {
        m_player.cells.fill(Water);
        m_player.ships.clear();
        m_setupHistory.clear();
        m_setupPlaced = QVector<bool>(m_setupLens.size(), false);
        m_setupSelectedIndex = 0;
        m_lastAction = QStringLiteral("Fleet cleared for no-touch rule");
        emit boardsChanged();
        emit statusChanged();
    }
    if (m_setupMode && v && !fleetSatisfiesNoTouchRule(m_enemy)) {
        placeFleetRandom(m_enemy);
    }
    emit setupChanged();
    scheduleSave();
}

qint64 GameEngine::elapsedMilliseconds() const
{
    return m_elapsedMillisecondsBase
        + (m_elapsedTimerRunning ? m_elapsedTimer.elapsed() : 0);
}

void GameEngine::updateElapsedSeconds()
{
    const int seconds = int(elapsedMilliseconds() / 1000);
    if (seconds == m_elapsedSeconds) return;
    m_elapsedSeconds = seconds;
    emit elapsedSecondsChanged();
}

void GameEngine::startElapsedTimer()
{
    if (m_elapsedTimerRunning || !m_applicationActive || m_setupMode
            || m_gameOver || !m_playerTurn) {
        return;
    }
    m_elapsedTimer.restart();
    m_elapsedTimerRunning = true;
    m_elapsedTick.start();
}

void GameEngine::stopElapsedTimer()
{
    if (m_elapsedTimerRunning) {
        m_elapsedMillisecondsBase += m_elapsedTimer.elapsed();
        m_elapsedTimerRunning = false;
    }
    m_elapsedTick.stop();
    updateElapsedSeconds();
}

void GameEngine::setApplicationActive(bool active)
{
    if (m_applicationActive == active) return;
    m_applicationActive = active;
    if (active) startElapsedTimer();
    else stopElapsedTimer();
}

void GameEngine::initSetupDefaults()
{
    // Canonical fleet: 5,4,3,3,2
    m_setupLens = QVector<int>({5, 4, 3, 3, 2});
    m_setupNames = QVector<QString>({QStringLiteral("Carrier"),
                                    QStringLiteral("Battleship"),
                                    QStringLiteral("Cruiser"),
                                    QStringLiteral("Submarine"),
                                    QStringLiteral("Destroyer")});
    if (m_setupPlaced.size() != m_setupLens.size())
        m_setupPlaced = QVector<bool>(m_setupLens.size(), false);
    if (m_setupSelectedIndex < 0 || m_setupSelectedIndex >= m_setupLens.size())
        m_setupSelectedIndex = 0;
}

void GameEngine::scheduleSave()
{
    // Persist immediately. If we ever want to debounce disk writes, this is the only place to change.
    if (m_booting) return;
    saveToDisk();
}

void GameEngine::newGame()
{
    m_setupHistory.clear();
    // Reset core game state
    // NOTE: Board() leaves vectors empty, so always initialize via resetBoards().
    resetBoards();

    
    aiReset();
m_gameOver = false;
    m_playerWon = false;
    m_revealEnemyFleet = false;

    m_lastPlayerShotX = m_lastPlayerShotY = -1;
    m_lastAiShotX = m_lastAiShotY = -1;

    // Timer reset (battle timer starts when Start battle is pressed)
    stopElapsedTimer();
    m_elapsedMillisecondsBase = 0;
    m_elapsedSeconds = 0;
    emit elapsedSecondsChanged();

    // Setup phase: enemy is placed, player must place manually (or Auto-place from menu)
    placeFleetRandom(m_enemy);

    // Prepare setup ship list
    initSetupDefaults();
    m_setupPlaced = QVector<bool>(m_setupLens.size(), false);
    m_setupSelectedIndex = 0;
    m_setupHorizontal = true;
    m_setupMode = true;

    m_statusText = QStringLiteral("Place your ships");
    m_playerShots = 0;
    m_lastAction = QStringLiteral("Tap your board to place the selected ship");
    m_playerTurn = true;

    emit revealEnemyFleetChanged();
    emit setupModeChanged();
    emit setupChanged();
    emitAllChanged();
    scheduleSave();
}


void GameEngine::reshufflePlayerShips()
{
    if (m_gameOver) return;

    bool anyShots = false;
    for (int v : m_enemy.cells) {
        if (v == Miss || v == Hit || v == Sunk) { anyShots = true; break; }
    }
    if (anyShots) {
        setStatus(m_statusText, QStringLiteral("Cannot reshuffle after shooting has started"));
        emit statusChanged();
        return;
    }

    m_player.cells.fill(Water);
    m_player.ships.clear();
    if (placeFleetRandom(m_player)) {
        setStatus(m_statusText, QStringLiteral("Your fleet reshuffled"));
    } else {
        setStatus(m_statusText, QStringLiteral("Fleet placement failed"));
    }
    emitAllChanged();
}

QVariantList GameEngine::playerCells() const
{
    QVariantList out;
    out.reserve(N*N);
    for (int i = 0; i < N*N; ++i) {
        int v = m_player.cells[i];
        if (v == Water) out.append(0);
        else out.append(v);
    }
    return out;
}

QVariantList GameEngine::enemyCells() const
{
    QVariantList out;
    out.reserve(N*N);

    for (int i = 0; i < N*N; ++i) {
        const int v = m_enemy.cells[i];

        if (!m_enemyRevealed[i]) {
            // Not fired yet: normally unknown, but allow game-over reveal to show intact ships.
            if (m_revealEnemyFleet && v == Ship) out.append(Ship);
            else out.append(Unknown);
            continue;
        }

        // Fired cell: show miss/hit.
        if (v == Water) out.append(Miss);
        else if (v == Ship) out.append(Hit);
        else out.append(v);
    }

    return out;
}

static int shipKindFromName(const QString &name)
{
    const QString n = name.toLower();
    if (n.contains(QStringLiteral("carrier"))) return GameEngine::Carrier;
    if (n.contains(QStringLiteral("battleship"))) return GameEngine::Battleship;
    if (n.contains(QStringLiteral("cruiser"))) return GameEngine::Cruiser;
    if (n.contains(QStringLiteral("submarine"))) return GameEngine::Submarine;
    if (n.contains(QStringLiteral("destroyer"))) return GameEngine::Destroyer;
    return GameEngine::NoShip;
}

QVariantList GameEngine::playerShipKinds() const
{
    QVariantList out;
    out.reserve(N*N);
    QVector<int> kinds(N*N, NoShip);
    for (const ShipInfo &s : m_player.ships) {
        const int kind = shipKindFromName(s.name);
        for (int cell : s.cells) {
            if (cell >= 0 && cell < kinds.size()) kinds[cell] = kind;
        }
    }
    for (int k : kinds) out.append(k);
    return out;
}

QVariantList GameEngine::enemyShipKinds() const
{
    QVariantList out;
    out.reserve(N*N);
    QVector<int> kinds(N*N, NoShip);
    for (const ShipInfo &s : m_enemy.ships) {
        const int kind = shipKindFromName(s.name);
        for (int cell : s.cells) {
            if (cell >= 0 && cell < kinds.size()) kinds[cell] = kind;
        }
    }
    for (int k : kinds) out.append(k);
    return out;
}

void GameEngine::resetBoards()
{
    m_player.cells = QVector<int>(N*N, Water);
    m_player.ships.clear();

    m_enemy.cells = QVector<int>(N*N, Water);
    m_enemy.ships.clear();

    if (m_enemyRevealed.size() != N*N)
        m_enemyRevealed = QVector<bool>(N*N, false);
    else
        std::fill(m_enemyRevealed.begin(), m_enemyRevealed.end(), false);
}

bool GameEngine::placeFleetRandom(Board &b)
{
    struct Def { int len; const char* name; };
    const Def fleet[] = {
        {5, "Carrier"},
        {4, "Battleship"},
        {3, "Cruiser"},
        {3, "Submarine"},
        {2, "Destroyer"},
    };

    // Restart the whole fleet when an individual ship cannot be placed. This
    // guarantees callers never receive a silently incomplete random fleet.
    for (int fleetAttempt = 0; fleetAttempt < 100; ++fleetAttempt) {
        b.cells = QVector<int>(N*N, Water);
        b.ships.clear();

        bool complete = true;
        for (const auto &s : fleet) {
            bool placed = false;
            for (int attempt = 0; attempt < 5000; ++attempt) {
                if (tryPlaceShip(b, s.len, QString::fromUtf8(s.name), m_noTouchRule)) {
                    placed = true;
                    break;
                }
            }
            if (!placed) {
                complete = false;
                break;
            }
        }
        if (complete) return true;
    }

    b.cells = QVector<int>(N*N, Water);
    b.ships.clear();
    return false;
}

bool GameEngine::fleetSatisfiesNoTouchRule(const Board &b) const
{
    if (b.cells.size() != N*N) return false;

    QVector<int> owner(N*N, -1);
    for (int shipIndex = 0; shipIndex < b.ships.size(); ++shipIndex) {
        const ShipInfo &ship = b.ships[shipIndex];
        for (int cell : ship.cells) {
            if (cell < 0 || cell >= N*N || owner[cell] != -1) return false;
            owner[cell] = shipIndex;
        }
    }

    for (int cell = 0; cell < owner.size(); ++cell) {
        if (owner[cell] < 0) continue;
        const int x = cell % N;
        const int y = cell / N;
        for (int ny = y - 1; ny <= y + 1; ++ny) {
            for (int nx = x - 1; nx <= x + 1; ++nx) {
                if (!inBounds(nx, ny)) continue;
                const int other = owner[idx(nx, ny)];
                if (other >= 0 && other != owner[cell]) return false;
            }
        }
    }
    return true;
}

bool GameEngine::tryPlaceShip(Board &b, int length, const QString &name, bool noTouchRule)
{
    bool horizontal = (randomBounded(2) == 0);
    int x = randomBounded(N);
    int y = randomBounded(N);

    if (!canPlaceAt(b, x, y, length, horizontal, noTouchRule))
        return false;

    applyShip(b, x, y, length, horizontal, name);
    return true;
}

bool GameEngine::canPlaceAt(const Board &b, int x, int y, int length, bool horizontal, bool noTouchRule) const
{
    int dx = horizontal ? 1 : 0;
    int dy = horizontal ? 0 : 1;
    int endx = x + dx * (length - 1);
    int endy = y + dy * (length - 1);
    if (!inBounds(endx, endy)) return false;

    for (int i = 0; i < length; ++i) {
        int cx = x + dx*i;
        int cy = y + dy*i;
        int ci = idx(cx, cy);
        if (b.cells[ci] != Water) return false;

        if (noTouchRule) {
            for (int ny = cy - 1; ny <= cy + 1; ++ny) {
                for (int nx = cx - 1; nx <= cx + 1; ++nx) {
                    if (!inBounds(nx, ny)) continue;
                    if (b.cells[idx(nx, ny)] == Ship) return false;
                }
            }
        }
    }
    return true;
}

void GameEngine::applyShip(Board &b, int x, int y, int length, bool horizontal, const QString &name)
{
    ShipInfo ship;
    ship.length = length;
    ship.name = name;

    int dx = horizontal ? 1 : 0;
    int dy = horizontal ? 0 : 1;

    for (int i = 0; i < length; ++i) {
        int cx = x + dx*i;
        int cy = y + dy*i;
        int ci = idx(cx, cy);
        b.cells[ci] = Ship;
        ship.cells.push_back(ci);
    }
    b.ships.push_back(ship);
}

GameEngine::ShipInfo* GameEngine::shipContaining(Board &b, int cellIndex)
{
    for (auto &s : b.ships) {
        for (int ci : s.cells) {
            if (ci == cellIndex) return &s;
        }
    }
    return nullptr;
}

bool GameEngine::resolveShot(Board &targetBoard, int x, int y, bool attackerIsPlayer, QString *outMessage, int *outSunkLen)
{
    if (outSunkLen) *outSunkLen = 0;
    if (!inBounds(x, y)) return false;

    int i = idx(x, y);
    int v = targetBoard.cells[i];

    if (v == Miss || v == Hit || v == Sunk) {
        if (outMessage) *outMessage = attackerIsPlayer
            ? QStringLiteral("Already shot there")
            : QStringLiteral("AI repeated a shot (should not happen)");
        return false;
    }

    if (v == Water) {
        targetBoard.cells[i] = Miss;
        if (outMessage) *outMessage = attackerIsPlayer ? QStringLiteral("Miss") : QStringLiteral("AI missed");
        return true;
    }

    if (v == Ship) {
        targetBoard.cells[i] = Hit;
        ShipInfo *ship = shipContaining(targetBoard, i);
        if (ship) {
            ship->hits += 1;
            if (ship->isSunk()) {
                if (outSunkLen) *outSunkLen = ship->length;
                for (int ci : ship->cells) targetBoard.cells[ci] = Sunk;
                if (outMessage) *outMessage = attackerIsPlayer
                    ? QStringLiteral("Sunk %1").arg(ship->name)
                    : QStringLiteral("AI sunk your %1").arg(ship->name);
            } else {
                if (outMessage) {
                    if (attackerIsPlayer) {
                        *outMessage = QStringLiteral("Hit %1").arg(ship ? ship->name : QStringLiteral("ship"));
                    } else {
                        *outMessage = QStringLiteral("AI hit your %1").arg(ship ? ship->name : QStringLiteral("ship"));
                    }
                }
            }
        } else {
            if (outMessage) {
                if (attackerIsPlayer) {
                    *outMessage = QStringLiteral("Hit %1").arg(ship ? ship->name : QStringLiteral("ship"));
                } else {
                    *outMessage = QStringLiteral("AI hit your %1").arg(ship ? ship->name : QStringLiteral("ship"));
                }
            }
        }
        return true;
    }

    return false;
}

void GameEngine::playerFire(int x, int y)
{
    if (m_gameOver) return;
    if (!m_playerTurn) return;
    if (!inBounds(x,y)) return;

    m_lastPlayerShotX = x;
    m_lastPlayerShotY = y;

    int i = idx(x,y);
    if (m_enemyRevealed[i]) {
        setStatus(m_statusText, QStringLiteral("Already fired there"));
        emit statusChanged();
        return;
    }

    stopElapsedTimer();
    m_enemyRevealed[i] = true;
    m_playerShots++;
    emit playerShotsChanged();

    QString msg;
    int sunkLen = 0;
    bool applied = resolveShot(m_enemy, x, y, true, &msg, &sunkLen);
    if (!applied) {
        setStatus(m_statusText, msg);
        emitAllChanged();
        return;
    }

    const int cellNow = m_enemy.cells[i];
    const bool hit = (cellNow == Hit || cellNow == Sunk);
    const bool sunk = (cellNow == Sunk);
    emit playerShotResolved(hit, sunk);

    bool enemyAllSunk = true;
    for (int v : m_enemy.cells) { if (v == Ship) { enemyAllSunk = false; break; } }
    if (enemyAllSunk) {
        m_playerWon = true;
    m_gameOver = true;
    stopElapsedTimer();
    emit gameFinished(true, m_elapsedSeconds);
        m_playerTurn = false;
        setStatus(QStringLiteral("Game over"), msg);
        emit boardsChanged();
        emit statusChanged();
        emit setupModeChanged();
        emit setupChanged();
        saveToDisk();
        return;
    }

    m_playerTurn = false;
    setStatus(QStringLiteral("AI turn"), msg);
    emitAllChanged();
    saveToDisk();
    QTimer::singleShot(m_aiDelayMs, this, [this]() { aiTakeTurn(); });
}

static void pushUnique(QVector<QPoint> &vec, const QPoint &p)
{
    if (vec.contains(p)) return;
    vec.push_back(p);
}

static QVector<int> aiDefaultRemainingLens()
{
    return QVector<int>{5,4,3,3,2};
}

void GameEngine::aiReset()
{
    std::fill(m_aiTried.begin(), m_aiTried.end(), false);
    m_aiTargets.clear();
    m_aiHits.clear();
    m_aiRemainingLens = aiDefaultRemainingLens();
}



QPoint GameEngine::aiChooseShot()
{
    // Remap difficulties:
    // 0 Easy   -> old Medium behavior
    // 1 Medium -> old Hard behavior
    // 2 Hard   -> heatmap/probability AI + strong target mode

    const int d = m_aiDifficulty;

    auto popTargetLIFO = [&]() -> QPoint {
        while (!m_aiTargets.isEmpty()) {
            QPoint p = m_aiTargets.takeLast();
            if (!inBounds(p.x(), p.y())) continue;
            if (m_aiTried[idx(p.x(), p.y())]) continue;
            return p;
        }
        return QPoint(-1,-1);
    };

    // --- Target mode ---
    if (d >= 1 && m_aiHits.size() >= 2) {
        // old Hard / new Hard: infer orientation and extend line
        QPoint a = m_aiHits[0];
        QPoint b = m_aiHits[1];
        int dx = (b.x() > a.x()) ? 1 : (b.x() < a.x() ? -1 : 0);
        int dy = (b.y() > a.y()) ? 1 : (b.y() < a.y() ? -1 : 0);

        if ((dx != 0 && dy == 0) || (dy != 0 && dx == 0)) {
            int minX = a.x(), maxX = a.x(), minY = a.y(), maxY = a.y();
            for (const QPoint &p : m_aiHits) {
                minX = std::min(minX, p.x());
                maxX = std::max(maxX, p.x());
                minY = std::min(minY, p.y());
                maxY = std::max(maxY, p.y());
            }
            QVector<QPoint> ext;
            ext.reserve(2);
            ext.push_back(QPoint(minX - dx, minY - dy));
            ext.push_back(QPoint(maxX + dx, maxY + dy));
            for (const QPoint &p : ext) {
                if (!inBounds(p.x(), p.y())) continue;
                if (m_aiTried[idx(p.x(), p.y())]) continue;
                return p;
            }
        }
    }

    if (d == 0) {
        // Easy (old Medium): consume targets LIFO if any
        QPoint t = popTargetLIFO();
        if (t.x() >= 0) return t;
    } else if (d == 1) {
        // Medium (old Hard): consume targets LIFO if any
        QPoint t = popTargetLIFO();
        if (t.x() >= 0) return t;
    } else {
        // Hard: if we have targets (from a hit), prefer them first
        QPoint t = popTargetLIFO();
        if (t.x() >= 0) return t;
    }

    // --- Hunt mode ---
    if (d <= 1) {
        // Easy/Medium hunt: checkerboard
        QVector<QPoint> candidates;
        candidates.reserve(N*N);
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                int ii = idx(x,y);
                if (m_aiTried[ii]) continue;
                if (((x + y) % 2) == 0) candidates.push_back(QPoint(x,y));
            }
        }
        if (!candidates.isEmpty()) {
            if (d == 1) {
                // Medium: slight center bias
                QPoint best = candidates[0];
                int bestScore = -1;
                for (const QPoint &p : candidates) {
                    int score = (N - std::abs(p.x() - (N/2))) + (N - std::abs(p.y() - (N/2)));
                    if (score > bestScore) { bestScore = score; best = p; }
                }
                return best;
            }
            return candidates[randomBounded(candidates.size())];
        }

        // fallback any
        QVector<QPoint> any;
        any.reserve(N*N);
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x)
                if (!m_aiTried[idx(x,y)]) any.push_back(QPoint(x,y));
        if (any.isEmpty()) return QPoint(0,0);
        return any[randomBounded(any.size())];
    }

    // Hard: probability heatmap hunt (and also works when no current target ship)
    QVector<int> score(N*N, 0);

    auto cellBlocksPlacement = [&](int x, int y) -> bool {
        int v = m_player.cells[idx(x,y)];
        return (v == Miss || v == Sunk); // Miss and already-sunk cells block remaining-ship placements
    };

    auto placementCompatibleWithHits = [&](int x0, int y0, int dx, int dy, int len) -> bool {
        // if we have active hits, placement must include all of them
        if (m_aiHits.isEmpty()) return true;
        for (const QPoint &h : m_aiHits) {
            bool inside = false;
            for (int k = 0; k < len; ++k) {
                int x = x0 + dx*k;
                int y = y0 + dy*k;
                if (x == h.x() && y == h.y()) { inside = true; break; }
            }
            if (!inside) return false;
        }
        return true;
    };

    auto addPlacement = [&](int x0, int y0, int dx, int dy, int len) {
        for (int k = 0; k < len; ++k) {
            int x = x0 + dx*k;
            int y = y0 + dy*k;
            int ii = idx(x,y);
            if (!m_aiTried[ii]) score[ii] += 1;
        }
    };

    const QVector<int> lens = m_aiRemainingLens.isEmpty() ? aiDefaultRemainingLens() : m_aiRemainingLens;

    for (int len : lens) {
        // horizontal placements
        for (int y = 0; y < N; ++y) {
            for (int x0 = 0; x0 <= N - len; ++x0) {
                bool ok = true;
                for (int k = 0; k < len; ++k) {
                    int x = x0 + k;
                    if (cellBlocksPlacement(x, y)) { ok = false; break; }
                    // if we have active hits, we must not contradict: hits are allowed; but we must not place over known water
                    int v = m_player.cells[idx(x,y)];
                    if (v == Water) { /* unknown */ }
                    if (v == Miss) { ok = false; break; }
                }
                if (!ok) continue;
                if (!placementCompatibleWithHits(x0, y, 1, 0, len)) continue;
                addPlacement(x0, y, 1, 0, len);
            }
        }

        // vertical placements
        for (int x = 0; x < N; ++x) {
            for (int y0 = 0; y0 <= N - len; ++y0) {
                bool ok = true;
                for (int k = 0; k < len; ++k) {
                    int y = y0 + k;
                    if (cellBlocksPlacement(x, y)) { ok = false; break; }
                    int v = m_player.cells[idx(x,y)];
                    if (v == Miss) { ok = false; break; }
                }
                if (!ok) continue;
                if (!placementCompatibleWithHits(x, y0, 0, 1, len)) continue;
                addPlacement(x, y0, 0, 1, len);
            }
        }
    }

    // choose best score among untried cells; tie-break by random
    int best = -1;
    QVector<int> bestIdx;
    bestIdx.reserve(N*N);
    for (int ii = 0; ii < N*N; ++ii) {
        if (m_aiTried[ii]) continue;
        int s = score[ii];
        if (s > best) {
            best = s;
            bestIdx.clear();
            bestIdx.push_back(ii);
        } else if (s == best) {
            bestIdx.push_back(ii);
        }
    }

    if (!bestIdx.isEmpty() && best > 0) {
        int ii = bestIdx[randomBounded(bestIdx.size())];
        int x = ii % N;
        int y = ii / N;
        return QPoint(x,y);
    }

    // fallback any
    QVector<QPoint> any;
    any.reserve(N*N);
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
            if (!m_aiTried[idx(x,y)]) any.push_back(QPoint(x,y));
    if (any.isEmpty()) return QPoint(0,0);
    return any[randomBounded(any.size())];
}

void GameEngine::aiNotifyResult(const QPoint &p, bool hit, bool sunk, int sunkLen)
{
    if (!hit) return;

    const int d = m_aiDifficulty;

    // Easy/Medium/Hard: track hits for Medium+ (old Hard and new Hard), also used by Hard heatmap constraints.
    if (d >= 1) {
        if (!m_aiHits.contains(p)) m_aiHits.push_back(p);
    }

    // Neighbor target list:
    const QPoint neigh[] = {
        QPoint(p.x()+1, p.y()),
        QPoint(p.x()-1, p.y()),
        QPoint(p.x(), p.y()+1),
        QPoint(p.x(), p.y()-1),
    };

    // Easy/Medium/Hard all push neighbors, but Easy (old Medium) and Medium (old Hard) are same here.
    for (const QPoint &n : neigh) {
        if (!inBounds(n.x(), n.y())) continue;
        int ii = idx(n.x(), n.y());
        if (m_aiTried[ii]) continue;
        pushUnique(m_aiTargets, n);
    }

    if (sunk) {
        // Remove sunk ship length from remaining lens (Hard only uses this, but keep it consistent)
        if (sunkLen > 0) {
            int i = m_aiRemainingLens.indexOf(sunkLen);
            if (i >= 0) m_aiRemainingLens.remove(i);
        }
        m_aiTargets.clear();
        m_aiHits.clear();
    }
}

void GameEngine::aiTakeTurn()
{
    // Ignore delayed callbacks that belong to a game which has since ended or
    // been replaced. A real AI turn is possible only during battle while the
    // player is locked out.
    if (m_gameOver || m_setupMode || m_playerTurn) return;

    QPoint shot = aiChooseShot();
    m_lastAiShotX = shot.x();
    m_lastAiShotY = shot.y();
    int ii = idx(shot.x(), shot.y());
    m_aiTried[ii] = true;

    QString msg;
    int sunkLen = 0;
    bool applied = resolveShot(m_player, shot.x(), shot.y(), false, &msg, &sunkLen);;
    if (!applied) {
        shot = aiChooseShot();
        m_lastAiShotX = shot.x();
        m_lastAiShotY = shot.y();
        ii = idx(shot.x(), shot.y());
        m_aiTried[ii] = true;
        resolveShot(m_player, shot.x(), shot.y(), false, &msg, &sunkLen);
    }

    int cellNow = m_player.cells[idx(shot.x(), shot.y())];
    bool hit = (cellNow == Hit || cellNow == Sunk);
    bool sunk = (cellNow == Sunk);
    aiNotifyResult(shot, hit, sunk, sunkLen);
    emit aiShotResolved(hit, sunk);

    bool playerAllSunk = true;
    for (int v : m_player.cells) { if (v == Ship) { playerAllSunk = false; break; } }
    if (playerAllSunk) {
        m_playerWon = false;
    m_gameOver = true;
    stopElapsedTimer();
    emit gameFinished(false, m_elapsedSeconds);
        m_playerTurn = false;
        setStatus(QStringLiteral("Game over"), msg);
        emit boardsChanged();
        emit statusChanged();
        emit setupModeChanged();
        emit setupChanged();
        saveToDisk();
        return;
    }

    m_playerTurn = true;
    setStatus(QStringLiteral("Your turn"), msg);
    emitAllChanged();
    saveToDisk();
    startElapsedTimer();
}

void GameEngine::setStatus(const QString &status, const QString &action)
{
    m_statusText = status;
    m_lastAction = action;
}

void GameEngine::emitAllChanged()
{
    emit boardsChanged();
    emit statusChanged();
    emit setupModeChanged();
    emit setupChanged();
}


int GameEngine::randomBounded(int upper) const
{
    if (upper <= 0) return 0;
    quint32 x = m_randomState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_randomState = x;
    return int(x % static_cast<quint32>(upper));
}

void GameEngine::seedRandom(quint32 seed)
{
    m_randomState = seed ? seed : 0x6d2b79f5u;
}


QString GameEngine::saveFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/save.json");
}

QString GameEngine::legacySaveFilePath() const
{
    return QDir::homePath()
        + QStringLiteral("/.local/share/harbour-navalbattle/save.json");
}

static QVariantList intVecToList(const QVector<int> &v)
{
    QVariantList out;
    out.reserve(v.size());
    for (int x : v) out.append(x);
    return out;
}

static QVector<int> listToIntVec(const QVariantList &l)
{
    QVector<int> out;
    out.reserve(l.size());
    for (const QVariant &v : l) out.append(v.toInt());
    return out;
}

static QVariantList boolVecToList(const QVector<bool> &v)
{
    QVariantList out;
    out.reserve(v.size());
    for (bool b : v) out.append(b);
    return out;
}

static QVector<bool> listToBoolVec(const QVariantList &l)
{
    QVector<bool> out;
    out.reserve(l.size());
    for (const QVariant &v : l) out.append(v.toBool());
    return out;
}

static QVariantList pointsToList(const QVector<QPoint> &pts)
{
    QVariantList out;
    out.reserve(pts.size());
    for (const QPoint &p : pts) {
        QVariantMap m;
        m.insert(QStringLiteral("x"), p.x());
        m.insert(QStringLiteral("y"), p.y());
        out.append(m);
    }
    return out;
}

static QVector<QPoint> listToPoints(const QVariantList &l)
{
    QVector<QPoint> out;
    out.reserve(l.size());
    for (const QVariant &v : l) {
        QVariantMap m = v.toMap();
        out.append(QPoint(m.value(QStringLiteral("x")).toInt(),
                          m.value(QStringLiteral("y")).toInt()));
    }
    return out;
}

QVariantMap GameEngine::toVariantMap() const
{
    QVariantMap m;
    m.insert(QStringLiteral("version"), 2);
    m.insert(QStringLiteral("boardN"), N);

    m.insert(QStringLiteral("playerCells"), intVecToList(m_player.cells));
    m.insert(QStringLiteral("enemyCells"), intVecToList(m_enemy.cells));

    // Ships (store only hits/length/name/cells so sunk detection remains consistent)
    QVariantList pShips;
    for (const auto &s : m_player.ships) {
        QVariantMap sm;
        sm.insert(QStringLiteral("length"), s.length);
        sm.insert(QStringLiteral("hits"), s.hits);
        sm.insert(QStringLiteral("name"), s.name);
        sm.insert(QStringLiteral("cells"), intVecToList(s.cells));
        pShips.append(sm);
    }
    QVariantList eShips;
    for (const auto &s : m_enemy.ships) {
        QVariantMap sm;
        sm.insert(QStringLiteral("length"), s.length);
        sm.insert(QStringLiteral("hits"), s.hits);
        sm.insert(QStringLiteral("name"), s.name);
        sm.insert(QStringLiteral("cells"), intVecToList(s.cells));
        eShips.append(sm);
    }
    m.insert(QStringLiteral("playerShips"), pShips);
    m.insert(QStringLiteral("enemyShips"), eShips);

    m.insert(QStringLiteral("enemyRevealed"), boolVecToList(m_enemyRevealed));
    m.insert(QStringLiteral("gameOver"), m_gameOver);
    m.insert(QStringLiteral("playerWon"), m_playerWon);
    m.insert(QStringLiteral("revealEnemyFleet"), m_revealEnemyFleet);
    m.insert(QStringLiteral("playerTurn"), m_playerTurn);
    m.insert(QStringLiteral("statusText"), m_statusText);
    m.insert(QStringLiteral("lastAction"), m_lastAction);

    m.insert(QStringLiteral("lastPlayerShotX"), m_lastPlayerShotX);
    m.insert(QStringLiteral("lastPlayerShotY"), m_lastPlayerShotY);
    m.insert(QStringLiteral("lastAiShotX"), m_lastAiShotX);
    m.insert(QStringLiteral("lastAiShotY"), m_lastAiShotY);

    const qint64 elapsedMs = elapsedMilliseconds();
    m.insert(QStringLiteral("elapsedMilliseconds"), elapsedMs);
    m.insert(QStringLiteral("elapsedSeconds"), int(elapsedMs / 1000));
    m.insert(QStringLiteral("playerShots"), m_playerShots);
    m.insert(QStringLiteral("setupMode"), m_setupMode);
    m.insert(QStringLiteral("setupPlaced"), setupPlaced());
    m.insert(QStringLiteral("setupSelectedIndex"), m_setupSelectedIndex);
    m.insert(QStringLiteral("setupHorizontal"), m_setupHorizontal);
    // AI
    m.insert(QStringLiteral("aiTried"), boolVecToList(m_aiTried));
    m.insert(QStringLiteral("aiTargets"), pointsToList(m_aiTargets));
    m.insert(QStringLiteral("aiHits"), pointsToList(m_aiHits));
    m.insert(QStringLiteral("aiRemainingLens"), intVecToList(m_aiRemainingLens));
    m.insert(QStringLiteral("aiDifficulty"), m_aiDifficulty);
    m.insert(QStringLiteral("randomState"), static_cast<qulonglong>(m_randomState));

    return m;
}

bool GameEngine::fromVariantMap(const QVariantMap &m)
{
    const int ver = m.value(QStringLiteral("version")).toInt();
    if (ver != 2) return false;
    if (m.value(QStringLiteral("boardN")).toInt() != N) return false;

    // Ensure setup-related vectors have sane defaults even when loading older saves.
    initSetupDefaults();

    QVector<int> pc = listToIntVec(m.value(QStringLiteral("playerCells")).toList());
    QVector<int> ec = listToIntVec(m.value(QStringLiteral("enemyCells")).toList());
    if (pc.size() != N*N || ec.size() != N*N) return false;

    m_player.cells = pc;
    m_enemy.cells = ec;

    auto loadShips = [](const QVariantList &shipsList, QVector<GameEngine::ShipInfo> &shipsOut) {
        shipsOut.clear();
        shipsOut.reserve(shipsList.size());
        for (const QVariant &v : shipsList) {
            QVariantMap sm = v.toMap();
            GameEngine::ShipInfo s;
            s.length = sm.value(QStringLiteral("length")).toInt();
            s.hits = sm.value(QStringLiteral("hits")).toInt();
            s.name = sm.value(QStringLiteral("name")).toString();
            s.cells = listToIntVec(sm.value(QStringLiteral("cells")).toList());
            // Basic validation
            if (s.length <= 0 || s.cells.size() != s.length) return false;
            shipsOut.push_back(s);
        }
        return true;
    };

    if (!loadShips(m.value(QStringLiteral("playerShips")).toList(), m_player.ships)) return false;
    if (!loadShips(m.value(QStringLiteral("enemyShips")).toList(), m_enemy.ships)) return false;

    m_enemyRevealed = listToBoolVec(m.value(QStringLiteral("enemyRevealed")).toList());
    if (m_enemyRevealed.size() != N*N) return false;

    m_gameOver = m.value(QStringLiteral("gameOver")).toBool();
    m_playerTurn = m.value(QStringLiteral("playerTurn")).toBool();
    m_statusText = m.value(QStringLiteral("statusText")).toString();
    m_lastAction = m.value(QStringLiteral("lastAction")).toString();

    m_lastPlayerShotX = m.value(QStringLiteral("lastPlayerShotX"), -1).toInt();
    m_lastPlayerShotY = m.value(QStringLiteral("lastPlayerShotY"), -1).toInt();
    m_lastAiShotX = m.value(QStringLiteral("lastAiShotX"), -1).toInt();
    m_lastAiShotY = m.value(QStringLiteral("lastAiShotY"), -1).toInt();
    m_playerWon = m.value(QStringLiteral("playerWon"), false).toBool();
    m_revealEnemyFleet = m.value(QStringLiteral("revealEnemyFleet"), false).toBool();

    // Restore / initialize manual setup state
    m_setupPlaced = QVector<bool>(m_setupLens.size(), false);
    m_setupMode = m.value(QStringLiteral("setupMode"), false).toBool();
    m_setupSelectedIndex = m.value(QStringLiteral("setupSelectedIndex"), 0).toInt();
    m_setupHorizontal = m.value(QStringLiteral("setupHorizontal"), true).toBool();
    {
        const QVariantList placedList = m.value(QStringLiteral("setupPlaced")).toList();
        if (!placedList.isEmpty()) {
            QVector<bool> tmp;
            tmp.reserve(placedList.size());
            for (const QVariant &v : placedList) tmp.push_back(v.toBool());
            if (tmp.size() == m_setupPlaced.size()) m_setupPlaced = tmp;
        }
    }
    if (m_setupSelectedIndex < 0) m_setupSelectedIndex = 0;
    if (m_setupSelectedIndex >= m_setupLens.size()) m_setupSelectedIndex = m_setupLens.size() - 1;
    emit setupModeChanged();
    emit setupChanged();

    m_aiTried = listToBoolVec(m.value(QStringLiteral("aiTried")).toList());
    if (m_aiTried.size() != N*N) return false;
    m_aiTargets = listToPoints(m.value(QStringLiteral("aiTargets")).toList());
    m_aiHits = listToPoints(m.value(QStringLiteral("aiHits")).toList());
    m_aiRemainingLens = m.contains(QStringLiteral("aiRemainingLens"))
        ? listToIntVec(m.value(QStringLiteral("aiRemainingLens")).toList())
        : aiDefaultRemainingLens();
    m_aiDifficulty = qBound(0,
        m.value(QStringLiteral("aiDifficulty"), m_aiDifficulty).toInt(), 2);

    auto pointsAreValid = [this](const QVector<QPoint> &points) {
        for (const QPoint &p : points) {
            if (!inBounds(p.x(), p.y())) return false;
        }
        return true;
    };
    if (!pointsAreValid(m_aiTargets) || !pointsAreValid(m_aiHits)) return false;
    if (m_aiRemainingLens.size() > 5) return false;
    for (int length : m_aiRemainingLens) {
        if (length < 2 || length > 5) return false;
    }
    seedRandom(static_cast<quint32>(
        m.value(QStringLiteral("randomState"), 0x6d2b79f5u).toULongLong()));

    // Restore timer state
    m_elapsedTimerRunning = false;
    m_elapsedTick.stop();
    m_elapsedMillisecondsBase = m.contains(QStringLiteral("elapsedMilliseconds"))
        ? qMax<qint64>(0, m.value(QStringLiteral("elapsedMilliseconds")).toLongLong())
        : qint64(qMax(0, m.value(QStringLiteral("elapsedSeconds"), 0).toInt())) * 1000;
    m_elapsedSeconds = int(m_elapsedMillisecondsBase / 1000);
    m_playerShots = m.value(QStringLiteral("playerShots"), 0).toInt();
    emit playerShotsChanged();
    startElapsedTimer();
    emit elapsedSecondsChanged();

    return true;
}

bool GameEngine::saveToDisk()
{
    if (m_booting) {
        return false;
    }

    updateElapsedSeconds();
    const QString path = saveFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    const QVariantMap m = toVariantMap();
    const QJsonObject obj = QJsonObject::fromVariantMap(m);
    const QJsonDocument doc(obj);

    const QByteArray bytes = doc.toJson(QJsonDocument::Compact);
    const qint64 written = f.write(bytes);
    f.close();
    return (written == bytes.size());
}



bool GameEngine::loadFromDisk()
{
    const QString path = saveFilePath();
    if (loadFromPath(path)) {
        return true;
    }

    // One-time compatibility with releases that used a hard-coded home path.
    // Sailjail may make that old location unavailable; failure is harmless.
    const QString legacyPath = legacySaveFilePath();
    if (legacyPath != path && loadFromPath(legacyPath)) {
        saveToDisk();
        return true;
    }

    m_booting = false;
    return false;
}

bool GameEngine::loadFromPath(const QString &path)
{
    QFile f(path);
    if (!f.exists()) return false;

    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QVariantMap m = doc.object().toVariantMap();
    m_booting = true;
    const bool ok = fromVariantMap(m);
    m_booting = false;
    if (!ok) {
        if (path == saveFilePath()) {
            QFile::remove(path);
        }
        return false;
    }

    // If the app was stopped after the player fired but before the delayed AI
    // move ran, resume that pending turn instead of leaving the board locked.
    if (!m_gameOver && !m_setupMode && !m_playerTurn) {
        QTimer::singleShot(m_aiDelayMs, this, [this]() { aiTakeTurn(); });
    }
    return true;
}



bool GameEngine::clearSavedGame()
{
    const QString path = saveFilePath();
    QFile f(path);
    if (f.exists() && !f.remove()) {
        return false;
    }

    // Clear + immediately start a new game so the cleared state is not re-saved.
    newGame();
    saveToDisk();

    return true;
}

void GameEngine::setRevealEnemyFleet(bool on)
{
    if (m_revealEnemyFleet == on) return;
    m_revealEnemyFleet = on;
    // Avoid "single-cell ship" confusion (last-shot highlight)
    m_lastPlayerShotX = m_lastPlayerShotY = -1;
    m_lastAiShotX = m_lastAiShotY = -1;
    emit revealEnemyFleetChanged();
    emitAllChanged();
}


QVariantList GameEngine::setupPlaced() const
{
    QVariantList out;
    out.reserve(m_setupPlaced.size());
    for (bool b : m_setupPlaced) out.append(b);
    return out;
}

void GameEngine::setSetupSelectedIndex(int idx)
{
    if (idx < 0) idx = 0;
    if (idx >= m_setupLens.size()) idx = m_setupLens.size() - 1;
    if (m_setupSelectedIndex == idx) return;
    m_setupSelectedIndex = idx;
    emit setupChanged();
}

void GameEngine::setSetupHorizontal(bool h)
{
    if (m_setupHorizontal == h) return;
    m_setupHorizontal = h;
    emit setupChanged();
}

void GameEngine::rotateSetupShip()
{
    setSetupHorizontal(!m_setupHorizontal);
}


bool GameEngine::undoSetupShip()
{
    if (!m_setupMode) return false;
    if (m_setupHistory.isEmpty()) return false;

    const SetupPlacement last = m_setupHistory.takeLast();
    if (last.shipIndex < 0 || last.shipIndex >= m_setupPlaced.size()) return false;

    // Clear cells from player board
    const int dx = last.horizontal ? 1 : 0;
    const int dy = last.horizontal ? 0 : 1;
    QVector<int> cells;
    cells.reserve(last.len);
    for (int i = 0; i < last.len; ++i) {
        const int cx = last.x + dx * i;
        const int cy = last.y + dy * i;
        if (!inBounds(cx, cy)) continue;
        const int ci = idx(cx, cy);
        cells.push_back(ci);
        if (m_player.cells[ci] == Ship)
            m_player.cells[ci] = Water;
    }

    // Remove the corresponding ShipInfo from m_player.ships (match by name+cells)
    for (int si = 0; si < m_player.ships.size(); ++si) {
        const auto &s = m_player.ships[si];
        if (s.name != last.name || s.length != last.len) continue;
        // Compare cell sets (order-insensitive)
        bool match = (s.cells.size() == cells.size());
        if (match) {
            for (int c : cells) {
                if (!s.cells.contains(c)) { match = false; break; }
            }
        }
        if (match) {
            m_player.ships.removeAt(si);
            break;
        }
    }

    m_setupPlaced[last.shipIndex] = false;
    m_setupSelectedIndex = last.shipIndex;
    m_lastAction = QStringLiteral("Undid %1").arg(last.name);
    emit setupChanged();
    emitAllChanged();
    emit statusChanged();
    scheduleSave();
    return true;
}

void GameEngine::rebuildSetupHistory()
{
    m_setupHistory.clear();

    // Reconstruct from current player fleet (setup ships are stored in m_player.ships).
    for (int shipIndex = 0; shipIndex < m_player.ships.size(); ++shipIndex) {
        const ShipInfo &s = m_player.ships[shipIndex];
        if (s.cells.isEmpty()) continue;

        int minX = N, minY = N, maxX = -1, maxY = -1;
        for (int cell : s.cells) {
            if (cell < 0 || cell >= N*N) continue;
            const int x = cell % N;
            const int y = cell / N;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }

        SetupPlacement p;
        p.shipIndex = shipIndex;
        p.x = minX;
        p.y = minY;
        p.len = s.length;
        p.horizontal = (maxX - minX) >= (maxY - minY);
        p.name = s.name;
        m_setupHistory.push_back(p);
    }

    emit setupChanged();
}


bool GameEngine::placeSetupShip(int shipIndex, int x, int y)
{
    if (!m_setupMode) return false;
    if (shipIndex < 0 || shipIndex >= m_setupLens.size()) return false;
    if (m_setupPlaced.value(shipIndex)) return false;

    const int len = m_setupLens[shipIndex];
    const QString name = m_setupNames[shipIndex];
    const bool horizontal = m_setupHorizontal;

    const int dx = horizontal ? 1 : 0;
    const int dy = horizontal ? 0 : 1;

    // canPlaceAt(board, x, y, length, horizontal, noTouchRule)
    // Passing dx/dy here breaks vertical placement when dx==0.
    // Use the same no-touch rule as auto-place and AI.
    if (!canPlaceAt(m_player, x, y, len, horizontal, m_noTouchRule)) {
        m_lastAction = QStringLiteral("Can't place ship here");
        emit statusChanged();
        return false;
    }

    applyShip(m_player, x, y, len, horizontal, name);

    SetupPlacement p;
    p.shipIndex = shipIndex;
    p.x = x;
    p.y = y;
    p.horizontal = horizontal;
    p.len = len;
    p.name = name;
    m_setupHistory.push_back(p);

    m_setupPlaced[shipIndex] = true;

    // Auto-advance selection to next unplaced ship
    for (int i = 0; i < m_setupPlaced.size(); ++i) {
        if (!m_setupPlaced[i]) { m_setupSelectedIndex = i; break; }
    }

    m_lastAction = QStringLiteral("Placed %1").arg(name);
    emitAllChanged();
    emit setupChanged();
    emit statusChanged();
    scheduleSave();
    return true;
}

void GameEngine::autoPlacePlayerFleet()
{
    m_setupHistory.clear();
    if (!m_setupMode) return;

    // Clear the existing (properly initialized) board.
    const int n = N;
    if (m_player.cells.size() != n * n)
        m_player.cells = QVector<int>(n * n, Water);
    else
        m_player.cells.fill(Water);
    m_player.ships.clear();

    const bool placed = placeFleetRandom(m_player);

    for (int i = 0; i < m_setupPlaced.size(); ++i) m_setupPlaced[i] = placed;
    // Remain in setup mode; player must press "Start battle".
    m_statusText = QStringLiteral("Place your ships");
    m_lastAction = placed
        ? QStringLiteral("Fleet auto-placed")
        : QStringLiteral("Fleet placement failed");

    emit setupChanged();
    emitAllChanged();
    scheduleSave();
}


void GameEngine::startBattle()
{
    if (!m_setupMode) return;
    for (bool b : m_setupPlaced) {
        if (!b) {
            m_lastAction = QStringLiteral("Place all ships first");
            emit statusChanged();
            return;
        }
    }

    // Start the fair ranked clock only after setup has ended.
    stopElapsedTimer();
    m_elapsedMillisecondsBase = 0;
    m_elapsedSeconds = 0;
    emit elapsedSecondsChanged();

    m_setupMode = false;
    m_playerTurn = true;
    startElapsedTimer();
    m_statusText = QStringLiteral("Your turn");
    m_lastAction = QStringLiteral("Battle started");
    emit setupModeChanged();
    emit setupChanged();
    emitAllChanged();
    scheduleSave();
}
