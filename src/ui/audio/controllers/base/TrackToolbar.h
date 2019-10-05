#pragma once

#include <QTime>
#include <QString>

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSlider>
#include <QToolButton>

#include "src/helpers/DurationHelper.hpp"

class TrackToolbar : public QWidget {
    
    Q_OBJECT

    public:
        enum Action { Rewind, Forward, Play, Pause };

        TrackToolbar(QWidget* parent = nullptr);
    
        void updatePlayerPosition(int posInSeconds);
        void newTrack(int lengthInSeconds);
        void endTrack();

    signals:
        void actionRequired(const TrackToolbar::Action &action);
        void seeking(int posInSecs);

    private:
        static inline const QString _defaultNoTime = "--"; 
        static inline const QString _trackPlayStateTemplator = " %1 / %2 ";

        QToolButton* _playBtn;
        QIcon _playIcon = QIcon(":/icons/app/audio/play.png");
        QIcon _pauseIcon = QIcon(":/icons/app/audio/pause.png");

        QToolButton* _rewindBtn;
        QToolButton* _forwardBtn;

        QSlider* _trackStateSlider;
        QLabel* _trackPlayStateLbl;
        QString _playStateDescriptor; 

        void _setPlayButtonState(bool isPlaying);
        void _tooglePlayButtonState();
        void _updateTrackTimeStateDescriptor(int stateInSeconds);

        bool _sliderDown = false;

};