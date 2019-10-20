#pragma once

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <QFileInfo>

#include "RPZStatusLabel.h"
#include "ClientActivityIndicator.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/_others/ConnectivityObserver.h"
#include "DownloadStatus.hpp"

class RPZStatusBar : public QStatusBar, public ConnectivityObserver {

    Q_OBJECT

    public:
        RPZStatusBar(QWidget * parent = nullptr);

        void bindServerIndicators();

    public slots:
        void updateServerStateLabel(const QString &stateText, SLState state);
        void updateUPnPLabel(const QString &stateText, SLState state);
        void updateExtIPLabel(const QString &stateText, SLState state);
        void updateMapFileLabel(const QString &mapDescriptor, bool isMapDirty);

    private:
        RPZStatusLabel* _extIpLabel = nullptr;
        RPZStatusLabel* _upnpStateLabel = nullptr;
        RPZStatusLabel* _serverStateLabel = nullptr;
        RPZStatusLabel* _mapFileLabel = nullptr;
        ClientActivityIndicator* _activityIndicators = nullptr;
        DownloadStatus* _dlStatus = nullptr;

        void _installComponents();
        void _installLayout();

        virtual void connectingToServer() override;
        virtual void connectionClosed() override;

}; 