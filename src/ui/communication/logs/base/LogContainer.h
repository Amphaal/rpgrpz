#pragma once

#include <QLabel>
#include <QHBoxLayout>
#include <QHash>
#include <QUuid>
#include <QString>
#include <QWidget>

#include "src/shared/models/entities/base/Serializable.hpp"

class LogItem : public QWidget {
    
    public:
        LogItem();
        int positionInLog(); 
        void setPositionInLog(int position);
        QHBoxLayout* horizontalLayout();

    private:
        int _position = -1;
        QHBoxLayout* _hLayout = nullptr;
};

class LogText : public QLabel {
    public:
        LogText(const QString &text);
};

class LogContainer : public QWidget {
    public:
        LogContainer(QWidget *parent = nullptr);
        void clearLines();
    
    protected:
        LogItem* _getLine(Serializable &element);
        LogItem* _getLine(QUuid &elementId);
        LogItem* _addLine(Serializable &element, QUuid &putUnder = QUuid());
    
    private:
        QHash<QUuid, LogItem*> _linesBySerializableId;
        QVBoxLayout* _vLayout = nullptr;
};