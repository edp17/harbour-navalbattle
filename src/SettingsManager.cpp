#include "SettingsManager.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

namespace {
QString settingsFilePath()
{
    const QString directory = QStandardPaths::writableLocation(
        QStandardPaths::AppConfigLocation);
    QDir().mkpath(directory);
    return QDir(directory).filePath(QStringLiteral("settings.ini"));
}

QString bestTimesFilePath()
{
    const QString directory = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir().mkpath(directory);
    return QDir(directory).filePath(QStringLiteral("best-times.json"));
}
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(settingsFilePath(), QSettings::IniFormat)
{
    // QSettings' default single-file location is outside the directory granted
    // by Sailjail. Import anything still readable, then use the explicit file.
    if (!m_settings.value(QStringLiteral("explicitSettingsFileMigrated"), false).toBool()) {
        QSettings legacy(QStringLiteral("harbour"),
                         QStringLiteral("harbour-navalbattle"));
        const QStringList keys = legacy.allKeys();
        for (const QString &key : keys) {
            if (!m_settings.contains(key)) m_settings.setValue(key, legacy.value(key));
        }
        m_settings.setValue(QStringLiteral("explicitSettingsFileMigrated"), true);
        m_settings.sync();
    }

    // Phase 22 migration: first run on 12×12 discards old best times.
    // (This build is permanently 12×12; only do this once.)
    if (!m_settings.value(QStringLiteral("phase22_migrated_12x12"), false).toBool()) {
        m_settings.remove(QStringLiteral("bestTimes"));
        m_settings.remove(QStringLiteral("bestTimesJson"));
        m_settings.setValue(QStringLiteral("phase22_migrated_12x12"), true);
    }

    // RC3 changes ranked time from wall-clock battle duration to player-turn
    // thinking time. Older records cannot be compared fairly, so reset once.
    if (!m_settings.value(QStringLiteral("fairPlayerClockMigrated"), false).toBool()) {
        m_settings.remove(QStringLiteral("bestTimes"));
        m_settings.remove(QStringLiteral("bestTimesJson"));
        m_settings.setValue(QStringLiteral("fairPlayerClockMigrated"), true);
    }

    m_settings.sync();
    loadBestTimes();
}

bool SettingsManager::showCoordinates() const
{
    return m_settings.value(QStringLiteral("showCoordinates"), true).toBool();
}

bool SettingsManager::hapticFeedback() const
{
    return m_settings.value(QStringLiteral("hapticFeedback"), true).toBool();
}

void SettingsManager::setHapticFeedback(bool v)
{
    if (hapticFeedback() == v) return;
    m_settings.setValue(QStringLiteral("hapticFeedback"), v);
    emit changed();
}

bool SettingsManager::soundEffects() const
{
    return m_settings.value(QStringLiteral("soundEffects"), true).toBool();
}

void SettingsManager::setSoundEffects(bool v)
{
    if (soundEffects() == v) return;
    m_settings.setValue(QStringLiteral("soundEffects"), v);
    emit changed();
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


QVariantList SettingsManager::bestTimes() const
{
    return m_bestTimes;
}

void SettingsManager::loadBestTimes()
{
    bool loadedFromFile = false;
    QFile file(bestTimesFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error == QJsonParseError::NoError && document.isArray()) {
            m_bestTimes = document.array().toVariantList();
            loadedFromFile = true;
        }
    }

    // Import the RC2/RC3 QSettings representation when no data file exists.
    bool loadedSettingsJson = false;
    const QVariant storedJson = m_settings.value(QStringLiteral("bestTimesJson"));
    const QByteArray bytes = storedJson.type() == QVariant::ByteArray
        ? storedJson.toByteArray() : storedJson.toString().toUtf8();
    if (!loadedFromFile && !bytes.isEmpty()) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
        if (error.error == QJsonParseError::NoError && document.isArray()) {
            m_bestTimes = document.array().toVariantList();
            loadedSettingsJson = true;
        }
    }

    // Import records written by version 1.0 and early sandbox builds.
    if (!loadedFromFile && !loadedSettingsJson) {
        m_bestTimes = m_settings.value(QStringLiteral("bestTimes"), QVariantList()).toList();
    }

    QVariantList normalized;
    normalized.reserve(m_bestTimes.size());
    for (const QVariant &value : m_bestTimes) {
        QVariantMap entry;
        if (value.type() == QVariant::Int || value.type() == QVariant::LongLong) {
            QVariantMap m;
            m.insert(QStringLiteral("name"), QString());
            m.insert(QStringLiteral("seconds"), value.toInt());
            m.insert(QStringLiteral("difficulty"), 1);
            entry = m;
        } else if (value.type() == QVariant::Map) {
            entry = value.toMap();
        } else {
            continue;
        }

        if (!entry.contains(QStringLiteral("difficulty"))) {
            entry.insert(QStringLiteral("difficulty"), 1);
        }
        if (!entry.contains(QStringLiteral("pace"))) {
            entry.insert(QStringLiteral("pace"), -1);
        }
        normalized.append(entry);
    }
    m_bestTimes = normalized;

    if (!loadedFromFile || m_settings.contains(QStringLiteral("bestTimes"))
            || m_settings.contains(QStringLiteral("bestTimesJson"))) {
        saveBestTimes();
    }
}

void SettingsManager::saveBestTimes()
{
    const QJsonDocument document(QJsonArray::fromVariantList(m_bestTimes));
    const QByteArray json = document.toJson(QJsonDocument::Compact);

    QSaveFile file(bestTimesFilePath());
    const bool saved = file.open(QIODevice::WriteOnly)
        && file.write(json) == json.size()
        && file.commit();
    if (saved) {
        m_settings.remove(QStringLiteral("bestTimes"));
        m_settings.remove(QStringLiteral("bestTimesJson"));
    } else {
        // Retain a recoverable fallback if the dedicated file cannot be written.
        m_settings.setValue(QStringLiteral("bestTimesJson"),
                            QString::fromUtf8(json));
    }
    m_settings.sync();
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

    QVariantList list = m_bestTimes;

    QVariantMap entry;
    entry.insert(QStringLiteral("name"), playerName.trimmed());
    entry.insert(QStringLiteral("seconds"), elapsedSeconds);
    entry.insert(QStringLiteral("shots"), shots);
    entry.insert(QStringLiteral("difficulty"), aiDifficulty());
    entry.insert(QStringLiteral("pace"), aiPace());
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

    m_bestTimes = list;
    saveBestTimes();
    emit bestTimesChanged();
    emit changed();
}

void SettingsManager::clearBestTimes()
{
    m_bestTimes.clear();
    QFile::remove(bestTimesFilePath());
    m_settings.remove(QStringLiteral("bestTimes"));
    m_settings.remove(QStringLiteral("bestTimesJson"));
    m_settings.sync();
    emit bestTimesChanged();
    emit changed();
}
