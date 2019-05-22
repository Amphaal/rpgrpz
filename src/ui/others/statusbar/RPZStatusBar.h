#pragma once

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>

#include "RPZStatusLabel.hpp"

class RPZStatusBar : public QStatusBar {

    Q_OBJECT

    public:
        RPZStatusBar(QWidget * parent = nullptr);

        void updateSentAudioKbps(double kbps, int clients);
        void updateReceivedAudioKbps(double kbps);
        void updateServerStateLabel(const std::string &stateText, int state);
        void updateUPnPLabel(const std::string &stateText, int state);
        void updateExtIPLabel(const std::string &stateText, int state);
        void updateMapFileLabel(const QString &filePath, bool isDirty);

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