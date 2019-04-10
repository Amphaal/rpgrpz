#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVector>
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
        QHash<QUuid, QWidget*> _lines;
    
    public slots:
        QUuid writeAtEnd(const std::string & newMessage, QPalette* colorPalette = nullptr, QPixmap* pixAsIcon = nullptr, const QUuid &id = NULL);
        void removeLine(const QUuid &idToRemove);
};