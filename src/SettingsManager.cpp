#include "SettingsManager.h"

#include <algorithm>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("harbour"), QStringLiteral("harbour-navalbattle"))
{
}

bool SettingsManager::showCoordinates() const
{
    return m_settings.value(QStringLiteral("showCoordinates"), true).toBool();
}

void SettingsManager::setShowCoordinates(bool v)
{
    if (showCoordinates() == v)
        return;
    m_settings.setValue(QStringLiteral("showCoordinates"), v);
    emit changed();
}

int SettingsManager::aiDelayMs() const
{
    return m_settings.value(QStringLiteral("aiDelayMs"), 350).toInt();
}

int SettingsManager::cellSizePx() const
{
    return m_settings.value(QStringLiteral("cellSizePx"), 0).toInt();
}

void SettingsManager::setAiDelayMs(int ms)
{
    if (ms < 0) ms = 0;
    if (aiDelayMs() == ms)
        return;
    m_settings.setValue(QStringLiteral("aiDelayMs"), ms);
    emit changed();
}

void SettingsManager::setCellSizePx(int px)
{
    if (px < 0) px = 0;
    if (px == cellSizePx()) return;
    m_settings.setValue(QStringLiteral("cellSizePx"), px);
    emit changed();
}


int SettingsManager::hitMissHoldMs() const
{
    return m_settings.value(QStringLiteral("hitMissHoldMs"), 1200).toInt();
}

int SettingsManager::thinkingHoldMs() const
{
    return m_settings.value(QStringLiteral("thinkingHoldMs"), 900).toInt();
}

int SettingsManager::aiDifficulty() const
{
    return m_settings.value(QStringLiteral("aiDifficulty"), 1).toInt();
}


int SettingsManager::aiPace() const
{
    // 0=Slow, 1=Normal, 2=Fast
    if (m_settings.contains(QStringLiteral("aiPace")))
        return m_settings.value(QStringLiteral("aiPace"), 1).toInt();

    // Migration: infer a reasonable default from existing aiDelayMs key if present
    const int d = m_settings.value(QStringLiteral("aiDelayMs"), 400).toInt();
    int pace = 1;
    if (d >= 600) pace = 0;
    else if (d <= 200) pace = 2;
    return pace;
}

void SettingsManager::setAiPace(int v)
{
    v = qBound(0, v, 2);
    m_settings.setValue(QStringLiteral("aiPace"), v);

    // Keep legacy keys updated (hidden from UI)
    if (v == 0) { // Slow
        m_settings.setValue(QStringLiteral("aiDelayMs"), 700);
        m_settings.setValue(QStringLiteral("hitMissHoldMs"), 1800);
        m_settings.setValue(QStringLiteral("thinkingHoldMs"), 700);
    } else if (v == 2) { // Fast
        m_settings.setValue(QStringLiteral("aiDelayMs"), 150);
        m_settings.setValue(QStringLiteral("hitMissHoldMs"), 700);
        m_settings.setValue(QStringLiteral("thinkingHoldMs"), 200);
    } else { // Normal
        m_settings.setValue(QStringLiteral("aiDelayMs"), 400);
        m_settings.setValue(QStringLiteral("hitMissHoldMs"), 1200);
        m_settings.setValue(QStringLiteral("thinkingHoldMs"), 400);
    }

    emit changed();
}

bool SettingsManager::noTouchRule() const
{
    return m_settings.value(QStringLiteral("noTouchRule"), false).toBool();
}

bool SettingsManager::showThinkingPopup() const
{
    return m_settings.value(QStringLiteral("showThinkingPopup"), true).toBool();
}


bool SettingsManager::showGameOverOverlay() const
{
    return m_settings.value(QStringLiteral("showGameOverOverlay"), true).toBool();
}

bool SettingsManager::showSunkShipSprites() const
{
    return m_settings.value(QStringLiteral("showSunkShipSprites"), true).toBool();
}

bool SettingsManager::tintEnemyShips() const
{
    return m_settings.value(QStringLiteral("tintEnemyShips"), false).toBool();
}


void SettingsManager::setHitMissHoldMs(int ms)
{
    if (ms < 0) ms = 0;
    if (hitMissHoldMs() == ms)
        return;
    m_settings.setValue(QStringLiteral("hitMissHoldMs"), ms);
    emit changed();
}

void SettingsManager::setThinkingHoldMs(int ms)
{
    if (ms < 0) ms = 0;
    if (thinkingHoldMs() == ms)
        return;
    m_settings.setValue(QStringLiteral("thinkingHoldMs"), ms);
    emit changed();
}

void SettingsManager::setAiDifficulty(int d)
{
    if (d < 0) d = 0;
    if (d > 2) d = 2;
    if (aiDifficulty() == d) return;
    m_settings.setValue(QStringLiteral("aiDifficulty"), d);
    emit changed();
}


void SettingsManager::setNoTouchRule(bool v)
{
    if (noTouchRule() == v) return;
    m_settings.setValue(QStringLiteral("noTouchRule"), v);
    emit changed();
}

void SettingsManager::setShowThinkingPopup(bool v)
{
    if (showThinkingPopup() == v) return;
    m_settings.setValue(QStringLiteral("showThinkingPopup"), v);
    emit changed();
}


void SettingsManager::setShowGameOverOverlay(bool v)
{
    m_settings.setValue(QStringLiteral("showGameOverOverlay"), v);
    emit changed();
}

void SettingsManager::setShowSunkShipSprites(bool v)
{
    if (showSunkShipSprites() == v) return;
    m_settings.setValue(QStringLiteral("showSunkShipSprites"), v);
    emit changed();
}

void SettingsManager::setTintEnemyShips(bool v)
{
    if (tintEnemyShips() == v) return;
    m_settings.setValue(QStringLiteral("tintEnemyShips"), v);
    emit changed();
}


QString SettingsManager::playerName() const
{
    return m_settings.value(QStringLiteral("playerName"), QString()).toString();
}

void SettingsManager::setPlayerName(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (playerName() == trimmed) return;
    m_settings.setValue(QStringLiteral("playerName"), trimmed);
    emit changed();
}


QVariantList SettingsManager::bestTimes()
{
    QVariantList list = m_settings.value(QStringLiteral("bestTimes"), QVariantList()).toList();
    // Migrate legacy format: list of ints -> list of {name, seconds}
    bool migrated = false;
    for (int i = 0; i < list.size(); ++i) {
        const QVariant &v = list.at(i);
        if (v.type() == QVariant::Int || v.type() == QVariant::LongLong) {
            QVariantMap m;
            m.insert(QStringLiteral("name"), QString());
            m.insert(QStringLiteral("seconds"), v.toInt());
            m.insert(QStringLiteral("difficulty"), 1);
            list[i] = m;
            migrated = true;
        }
    }
    if (migrated) {
        m_settings.setValue(QStringLiteral("bestTimes"), list);
    }

    // Ensure every entry has difficulty
    bool fixed = false;
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i).type() == QVariant::Map) {
            QVariantMap m = list.at(i).toMap();
            if (!m.contains(QStringLiteral("difficulty"))) {
                m.insert(QStringLiteral("difficulty"), 1);
                list[i] = m;
                fixed = true;
            }
        }
    }
    if (fixed) m_settings.setValue(QStringLiteral("bestTimes"), list);
    return list;
}

void SettingsManager::addBestTime(int elapsedSeconds)
{
    addBestTime(QString(), elapsedSeconds);
}


void SettingsManager::addBestTime(const QString &playerName, int elapsedSeconds)
{
    addBestTime(playerName, elapsedSeconds, 0);
}

void SettingsManager::addBestTime(const QString &playerName, int elapsedSeconds, int shots)
{
    if (elapsedSeconds <= 0) return;

    QVariantList list = bestTimes();

    QVariantMap entry;
    entry.insert(QStringLiteral("name"), playerName.trimmed());
    entry.insert(QStringLiteral("seconds"), elapsedSeconds);
    entry.insert(QStringLiteral("shots"), shots);
    entry.insert(QStringLiteral("difficulty"), aiDifficulty());
    list.append(entry);

    auto secondsOf = [](const QVariant &v) -> int {
        if (v.type() == QVariant::Map) return v.toMap().value(QStringLiteral("seconds")).toInt();
        return v.toInt();
    };

    // Sort ascending by seconds
    std::sort(list.begin(), list.end(), [&](const QVariant &a, const QVariant &b) {
        return secondsOf(a) < secondsOf(b);
    });

    // Keep top 10 per difficulty
    auto diffOf = [](const QVariant &v) -> int {
        if (v.type() == QVariant::Map) return v.toMap().value(QStringLiteral("difficulty")).toInt();
        return 1;
    };
    for (int d = 0; d <= 2; ++d) {
        int count = 0;
        for (int i = 0; i < list.size(); ) {
            if (diffOf(list.at(i)) == d) {
                ++count;
                if (count > 10) {
                    list.removeAt(i);
                    continue;
                }
            }
            ++i;
        }
    }

    m_settings.setValue(QStringLiteral("bestTimes"), list);
    emit changed();
}

void SettingsManager::clearBestTimes()
{
    m_settings.remove(QStringLiteral("bestTimes"));
    emit changed();
}