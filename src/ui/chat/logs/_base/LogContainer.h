#pragma once

#include <QLabel>
#include <QHBoxLayout>
#include <QHash>
#include <QString>
#include <QWidget>

#include "src/shared/models/_base/Stampable.hpp"

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
        LogItem* _getLine(const Stampable &element);
        LogItem* _getLine(Stampable::Id elementId);
        LogItem* _addLine(const Stampable &element, Stampable::Id putUnder = 0);
        QVBoxLayout* _getLayout();
    
    private:
        QHash<Stampable::Id, LogItem*> _linesBySerializableId;
        QVBoxLayout* _vLayout = nullptr;
};