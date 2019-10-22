#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/ui/audio/controllers/_base/VolumeToolbar.h"
#include "src/helpers/StringHelper.hpp"

class AudioProbeController : public QGroupBox {

    Q_OBJECT

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