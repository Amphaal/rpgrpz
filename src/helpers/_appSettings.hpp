#pragma once

#include <QSettings>

class AppSettings : public QSettings {
    public:
        AppSettings(const QString &path): QSettings(path, QSettings::IniFormat) {}

        int audioVolume() {
            return this->value(QStringLiteral(u"volume"), 50).toInt();
        }
        void setAudioVolume(int volume = 100) {
            this->setValue(QStringLiteral(u"volume"), volume);
        }

        int defaultLayer() {
            return this->value(QStringLiteral(u"defaultLayer"), 0).toInt();
        }
        void setDefaultLayer(int layer = 0) {
            this->setValue(QStringLiteral(u"defaultLayer"), layer);
        }

        bool gridActive() {
            return this->value(QStringLiteral(u"grid"), false).toBool();
        }

        bool scaleActive() {
            return this->value(QStringLiteral(u"scale"), false).toBool();
        }
};
