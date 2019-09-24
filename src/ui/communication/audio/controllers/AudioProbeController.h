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
#include "src/helpers/DurationHelper.hpp"

class AudioProbeController : public QGroupBox {

    public:
        AudioProbeController(QWidget * parent = nullptr);
        
        void updatePlayedMusic(const QString &musicName);
        void changeTrackState(bool isPlaying);
        void changeTrackPosition(int posInSecs);
        
        VolumeToolbar* toolbar = nullptr;
        
    private:
        static inline const QString _defaultNoTime = "--"; 

        QLabel* _descr = nullptr;
        QLabel* _playIconLbl = nullptr;
        QLabel* _posInTrackLbl = nullptr;
        QMovie* _playIcon = nullptr;
};