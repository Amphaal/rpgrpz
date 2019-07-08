#pragma once

#include <QTime>
#include <QString>

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSlider>
#include <QToolButton>

class TrackToolbar : public QWidget {
    
    Q_OBJECT

    public:
        enum Action { Rewind, Forward, Play, Pause };

        TrackToolbar(QWidget* parent = nullptr);
    
        void updateTrackState(int stateInSeconds);
        void newTrack(int lengthInSeconds);
        void endTrack();

    signals:
        void actionRequired(const TrackToolbar::Action &action);
        void seeking(int pos);

    private:
        QToolButton* _playBtn;
        QIcon _playIcon = QIcon(":/icons/app/audio/play.png");
        QIcon _pauseIcon = QIcon(":/icons/app/audio/pause.png");

        QToolButton* _rewindBtn;
        QToolButton* _forwardBtn;

        QSlider* _trackStateSlider;
        QLabel* _trackPlayStateLbl;
        static inline const QString _defaultNoTime = "--"; 
        QString _trackPlayStateTemplator = QString(" %1 / %2 ");
        QString _currentTrackEndFormated = TrackToolbar::_defaultNoTime; 

        QString _fromSecondsToTime(int lengthInSeconds);
        void _setPlayButtonState(bool isPlaying);
        void _tooglePlayButtonState();

        bool _sliderDown = false;
};