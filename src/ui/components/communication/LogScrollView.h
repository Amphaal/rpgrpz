#pragma once

#include "LogView.h"

#include <QScrollArea>
#include <QScrollBar>

class LogScrollView : public QScrollArea {

    Q_OBJECT

    public:
        LogScrollView(QWidget *parent = nullptr);

    public slots:
        void writeAtEnd(const std::string &newMessage, QPalette* colorPalette = nullptr, QPixmap* pixAsIcon = nullptr);
        void newLog();
        void clearLog();

    private:
        LogView* _log = nullptr;

        void _scrollUpdate();
};