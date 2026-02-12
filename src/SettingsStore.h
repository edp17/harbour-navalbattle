\
#pragma once

#include <QObject>
#include <QSettings>

class SettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showCoordinates READ showCoordinates WRITE setShowCoordinates NOTIFY showCoordinatesChanged)
    Q_PROPERTY(int aiDelayMs READ aiDelayMs WRITE setAiDelayMs NOTIFY aiDelayMsChanged)
    Q_PROPERTY(int cellSizePx READ cellSizePx WRITE setCellSizePx NOTIFY cellSizePxChanged)

public:
    explicit SettingsStore(QObject *parent = nullptr)
        : QObject(parent)
        , m_settings()
    {
    }

    bool showCoordinates() const { return m_settings.value(QStringLiteral("showCoordinates"), true).toBool(); }
    int aiDelayMs() const { return m_settings.value(QStringLiteral("aiDelayMs"), 350).toInt(); }
    int cellSizePx() const { return m_settings.value(QStringLiteral("cellSizePx"), 0).toInt(); }

public slots:
    void setShowCoordinates(bool v)
    {
        if (v == showCoordinates()) return;
        m_settings.setValue(QStringLiteral("showCoordinates"), v);
        emit showCoordinatesChanged();
    }

    void setAiDelayMs(int ms)
    {
        if (ms < 0) ms = 0;
        if (ms == aiDelayMs()) return;
        m_settings.setValue(QStringLiteral("aiDelayMs"), ms);
        emit aiDelayMsChanged();
    }

    void setCellSizePx(int px)
    {
        if (px < 0) px = 0;
        if (px == cellSizePx()) return;
        m_settings.setValue(QStringLiteral("cellSizePx"), px);
        emit cellSizePxChanged();
    }

signals:
    void showCoordinatesChanged();
    void aiDelayMsChanged();
    void cellSizePxChanged();

private:
    QSettings m_settings;
};
