#pragma once

#include <QSettings>

class AppSettings : public QSettings {
    public:
        AppSettings(const QString &path) : QSettings(path, QSettings::IniFormat) {}

        int audioVolume() {
            return this->value(QStringLiteral(u"volume"), 50).toInt();
        }
        void setAudioVolume(int volume = 100) {
            this->setValue(QStringLiteral(u"volume"), volume);
        }

        int defaultLayer() {
            return this->value(QStringLiteral(u"defaultLayer"), 0).toInt();
        }
        void setDefaultLayer(int layer) {
            this->setValue(QStringLiteral(u"defaultLayer"), layer);
        }

        bool hiddenAtomAsDefault() {
            return this->value(QStringLiteral(u"hiddenAsDefault"), false).toBool();
        }
        void setHiddenAtomAsDefault(bool hidden) {
            this->setValue(QStringLiteral(u"hiddenAsDefault"), hidden);
        }

        bool gridActive() {
            return this->value(QStringLiteral(u"grid"), false).toBool();
        }

        bool minimapActive() {
            return this->value(QStringLiteral(u"minimap"), false).toBool();
        }

        bool scaleActive() {
            return this->value(QStringLiteral(u"scale"), false).toBool();
        }
};
