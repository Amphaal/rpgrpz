#pragma once

#include "LogContainer.h"

#include <QScrollArea>
#include <QScrollBar>

class LogScrollView : public QScrollArea {

    Q_OBJECT

    public:
        LogScrollView(QWidget *parent = nullptr);

    public slots:
        void newLog();
        void clearLog();

    protected:
        QWidget* writeAtEnd(const QString &newMessage);

    private:
        LogContainer* _log = nullptr;
        void _scrollUpdate();
};