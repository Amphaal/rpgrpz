// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QSettings>

class AppSettings : public QSettings {
 public:
    explicit AppSettings(const QString &path) :
        QSettings(path, QSettings::IniFormat) {}

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

    void setMainWindowBarsShown(bool areShown) {
        this->setValue(QStringLiteral(u"mw_bars"), areShown);
    }

    bool mainWindowBarsShown() {
        return this->value(QStringLiteral(u"mw_bars"), true).toBool();
    }
};
