#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QList>
#include <QHash>
#include <QUuid>
#include <QTextEdit>
#include <QAction>
#include <QBitmap>
#include <QPixmap>

#include "src/helpers/_const.hpp"

class LogView : public QWidget {

    Q_OBJECT

    public:
        LogView(QWidget *parent = nullptr);
    
    public slots:
        QUuid writeAtEnd(const std::string & newMessage, QPalette* colorPalette = nullptr, QPixmap* pixAsIcon = nullptr);
        void removeLine(const QUuid &idToRemove);

    private:
        QHash<QUuid, QWidget*> _lines;
};