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

#include <QProgressBar>
#include <QLayout>
#include <QLocale>
#include <QStyleOptionProgressBar>

#include "src/ui/_others/ConnectivityObserver.h"

class DownloadStatus : public QProgressBar, public ConnectivityObserver {
    Q_OBJECT

 public:
    DownloadStatus() {
        this->setVisible(false);
        this->setContentsMargins(0, 0, 0, 0);
        this->setMinimum(0);
        this->setMaximumHeight(8);
    }

 protected:
    void connectingToServer() override {
        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONReceivingStarted,
            this, &DownloadStatus::_onDownloadStarted
        );

        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONDownloaded,
            this, &DownloadStatus::_onDownloadEnded
        );

        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONDownloading,
            this, &DownloadStatus::_onDownloadProgress
        );
    }
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override {
        this->_onDownloadEnded();
    }

 private slots:
    void _onDownloadStarted(RPZJSON::Method method, qint64 totalToDownload) {
        if (!RPZJSON::mayBeHeavyPayload(method)) return;

        this->_pendingMethod = method;

        this->setToolTip(QString());

        // at last set visible
        this->setMaximum((int)totalToDownload);
        this->setValue(0);
        this->setVisible(true);
    }

    void _onDownloadProgress(qint64 totalBytesDownloadedForBatch) {
        auto methodToStr = QVariant::fromValue(this->_pendingMethod).toString();
        auto locale = QLocale::system();
        auto pending = locale.formattedDataSize(totalBytesDownloadedForBatch);
        auto total = locale.formattedDataSize(this->maximum());

        auto toolTipText = this->_tooltipDescriptionTemplate
                            .arg(methodToStr)
                            .arg(pending)
                            .arg(total);
        this->setToolTip(toolTipText);

        this->setValue((int)totalBytesDownloadedForBatch);
    }

    void _onDownloadEnded() {
        this->setValue(this->maximum());
        this->setVisible(false);
    }

 private:
    static inline QString _tooltipDescriptionTemplate = QStringLiteral(u"%1 : %2/%3");

    RPZJSON::Method _pendingMethod = RPZJSON::Method::M_Unknown;
};
