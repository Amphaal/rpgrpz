#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

#include "src/shared/ClientBindable.hpp"

#include "AudioStreamToolbar.hpp"

class AudioStreamController : public QGroupBox, public ClientBindable {
    public:
        AudioStreamController(QWidget * parent = nullptr);
        void updatePlayedMusic(const QString &musicName);

        void bindToRPZClient(RPZClient * cc) override;
    
    private:
        AudioStreamToolbar* _toolbar;
};