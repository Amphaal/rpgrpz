#pragma once

#include <QLabel>
#include <QHBoxLayout>
#include <QHash>
#include <QString>
#include <QWidget>

#include "src/shared/models/base/Serializable.hpp"

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
        LogItem* _getLine(snowflake_uid elementId);
        LogItem* _addLine(Serializable &element, snowflake_uid putUnder = 0);
    
    private:
        QHash<snowflake_uid, LogItem*> _linesBySerializableId;
        QVBoxLayout* _vLayout = nullptr;
};