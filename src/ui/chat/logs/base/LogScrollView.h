#pragma once

#include "LogContainer.h"

#include <QScrollArea>
#include <QScrollBar>

class LogScrollView : public QScrollArea {
    public:
        LogScrollView(QWidget *parent = nullptr);

    private:
        void _scrollUpdate();
};