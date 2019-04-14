#pragma once

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

#include "src/shared/ClientBindable.hpp"

class AudioStreamController : public QGroupBox, public ClientBindable {
    public:
        AudioStreamController(QWidget * parent = nullptr);
        void updatePlayedMusic(const QString &musicName);

        void bindToRPZClient(RPZClient * cc) override;
};