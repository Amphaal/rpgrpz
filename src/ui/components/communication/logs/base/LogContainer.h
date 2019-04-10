#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QHash>
#include <QUuid>
#include <QString>
#include <QWidget>

class LogContainer : public QWidget {

    Q_OBJECT

    public:
        LogContainer(QWidget *parent = nullptr);
        QHash<QUuid, QWidget*> lines;
        QHash<QWidget*, QUuid> idOfLine;
    
    public slots:
        QWidget* writeAtEnd(const QString &newMessage);
        void removeLine(const QUuid &idToRemove);
};