#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

class AudioStreamNotifier : public QGroupBox {
    public:
        AudioStreamNotifier(QWidget * parent = nullptr);
        void updatePlayedMusic(const QString &musicName);
};