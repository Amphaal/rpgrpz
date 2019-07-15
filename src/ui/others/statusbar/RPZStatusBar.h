#pragma once

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>

#include "RPZStatusLabel.h"

class RPZStatusBar : public QStatusBar {

    public:
        RPZStatusBar(QWidget * parent = nullptr);

        void updateSentAudioKbps(double kbps, int clients);
        void updateReceivedAudioKbps(double kbps);
        void updateServerStateLabel(const QString &stateText, int state);
        void updateUPnPLabel(const QString &stateText, int state);
        void updateExtIPLabel(const QString &stateText, int state);
        void updateMapFileLabel(const QString &filePath, bool isMapDirty);

    private:
        RPZStatusLabel* _extIpLabel;
        RPZStatusLabel* _upnpStateLabel;
        RPZStatusLabel* _serverStateLabel;
        RPZStatusLabel* _sentAudioKbpsStateLabel;
        RPZStatusLabel* _receivedAudioKbpsStateLabel;
        QLabel* _mapFileLabel;

        void _installComponents();
        void _installLayout();
}; 