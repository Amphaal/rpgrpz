#pragma once

#include <QWidget>
#include <QSlider>
#include <QToolButton>
#include <QIcon>
#include <QLabel>

#include <QHBoxLayout>

#include "src/helpers/_appContext.h"

class VolumeToolbar : public QWidget {
    
    Q_OBJECT

    signals:
        void askForVolumeChange(int newVolume);

    public:
        VolumeToolbar(QWidget* parent = nullptr);
    
    private:
        QSlider* _audio = nullptr;
        QToolButton* _mute = nullptr;
        QLabel* _volumeStr = nullptr;

        QIcon _volumeIcon = QIcon(QStringLiteral(u":/icons/app/audio/volume.png"));
        QIcon _mutedIcon = QIcon(QStringLiteral(u":/icons/app/audio/mute.png"));

        void _onAudioChange(int newSliderVal);
        void _setAudioValLbl(int sliderVal);
        void _onMuteButtonClick();
};