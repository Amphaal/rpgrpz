#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QList>
#include <QMap>
#include <QUuid>

#include "src/helpers/_const.cpp"

class LogView : public QWidget {

    public:
        LogView(QWidget *parent = nullptr);
        QUuid writeAtEnd(const std::string & newMessage, QPalette* colorPalette = nullptr);
        void removeLine(QUuid idToRemove);

    private:
        QMap<QUuid, QLabel*> _labels;
};