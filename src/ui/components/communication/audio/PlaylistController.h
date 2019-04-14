#pragma once

#include <QGroupBox>

#include "src/shared/ClientBindable.hpp"

class PlaylistController : public QGroupBox, public ClientBindable {
    public:
        PlaylistController(QWidget * parent = nullptr);

        void bindToRPZClient(RPZClient * cc) override;
};