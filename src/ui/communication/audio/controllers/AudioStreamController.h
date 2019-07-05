#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

#include "src/ui/others/ClientBindable.h"

#include "base/AudioStreamToolbar.h"

class AudioStreamController : public QGroupBox, public ClientBindable {

    public:
        AudioStreamController(QWidget * parent = nullptr);
        void updatePlayedMusic(const QString &musicName);

        void onRPZClientConnecting(RPZClient * cc) override;
        void onRPZClientDisconnect(RPZClient* cc) override;
        
        AudioStreamToolbar* toolbar;
        
    private:
        QLabel* _descr = nullptr;
        QLabel* _playIconLbl = nullptr;
        QMovie* _playIcon = nullptr;
};