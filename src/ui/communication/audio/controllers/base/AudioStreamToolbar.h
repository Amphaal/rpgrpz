#pragma once

#include <QWidget>
#include <QSlider>
#include <QToolButton>
#include <QIcon>

#include <QHBoxLayout>

#include "src/helpers/_appContext.h"

class AudioStreamToolbar : public QWidget {
    
    Q_OBJECT

    signals:
        void askForVolumeChange(int newVolume);

    public:
        AudioStreamToolbar(QWidget* parent = nullptr);

        ~AudioStreamToolbar();
    
    private:
        QSlider* _audio;
        QToolButton* _mute;

        QIcon _volumeIcon = QIcon(":/icons/app/audio/volume.png");
        QIcon _mutedIcon = QIcon(":/icons/app/audio/mute.png");

        void _onAudioChange(int newSliderVal);

        void _onMuteButtonClick();
};