#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QList>
#include <QMap>
#include <QUuid>
#include <QTextEdit>
#include <QAction>
#include <QBitmap>
#include <QPixmap>

#include "src/helpers/_const.cpp"

class LogView : public QWidget {

    public:
        LogView(QWidget *parent = nullptr);
        QUuid writeAtEnd(const std::string & newMessage, QPalette* colorPalette = nullptr, QPixmap* pixAsIcon = nullptr);
        void removeLine(QUuid idToRemove);

    private:
        QMap<QUuid, QWidget*> _lines;
};