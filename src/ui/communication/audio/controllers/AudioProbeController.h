#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

#include "src/ui/others/ClientBindable.h"

#include "base/VolumeToolbar.h"

class AudioProbeController : public QGroupBox {

    public:
        AudioProbeController(QWidget * parent = nullptr);
        
        void updatePlayedMusic(const QString &musicName);
        void changeTrackState(bool isPlaying);
        
        VolumeToolbar* toolbar = nullptr;
        
    private:
        QLabel* _descr = nullptr;
        QLabel* _playIconLbl = nullptr;
        QMovie* _playIcon = nullptr;
};