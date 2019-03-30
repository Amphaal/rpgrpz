#pragma once

#include "LogView.h"

#include <QScrollArea>
#include <QScrollBar>

class LogScrollView : public QScrollArea {

    public:
        LogScrollView(QWidget *parent = nullptr);

        void writeAtEnd(const std::string & newMessage, QPalette* colorPalette = nullptr);

        void newLog();
        void clearLog();

    private:
        LogView* _log = 0;

        void _scrollUpdate();
};