#pragma once

#include <QTime>
#include <QString>

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSlider>
#include <QToolButton>

class PlaylistToolbar : public QWidget {
    
    Q_OBJECT

    public:
        enum Action { Rewind, Forward, Play, Pause };

        PlaylistToolbar(QWidget* parent = nullptr);
    
        void updateTrackState(int stateInSeconds);
        void newTrack(int lengthInSeconds);

    signals:
        void actionRequired(const PlaylistToolbar::Action &action);
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
        QString _currentTrackEndFormated = PlaylistToolbar::_defaultNoTime; 

        QString _fromSecondsToTime(int lengthInSeconds);
        void _setPlayButtonState(bool isPlaying);
        void _tooglePlayButtonState();
};