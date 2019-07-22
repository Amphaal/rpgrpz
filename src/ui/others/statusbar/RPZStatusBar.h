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

        void updateServerStateLabel(const QString &stateText, SLState state);
        void updateUPnPLabel(const QString &stateText, SLState state);
        void updateExtIPLabel(const QString &stateText, SLState state);
        void updateMapFileLabel(const QString &filePath, bool isMapDirty);

    private:
        RPZStatusLabel* _extIpLabel = nullptr;
        RPZStatusLabel* _upnpStateLabel = nullptr;
        RPZStatusLabel* _serverStateLabel = nullptr;
        RPZStatusLabel* _mapFileLabel = nullptr;

        void _installComponents();
        void _installLayout();
}; 