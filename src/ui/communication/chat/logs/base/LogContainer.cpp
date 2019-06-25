#include "LogContainer.h"

LogText::LogText(const QString &text) : QLabel(text) {
    this->setMargin(0);
    this->setWordWrap(true);
}

LogItem::LogItem() : QWidget(), _hLayout(new QHBoxLayout) {
    this->setLayout(this->_hLayout);
    this->layout()->setContentsMargins(10, 3, 10, 3);
    this->setAutoFillBackground(true);
}

int LogItem::positionInLog() {
    return this->_position;
}

void LogItem::setPositionInLog(int position) {
    this->_position = position;
}

QHBoxLayout* LogItem::horizontalLayout() {
    return this->_hLayout;
}

LogContainer::LogContainer(QWidget *parent) : QWidget(parent), _vLayout(new QVBoxLayout) {
    this->setLayout(this->_vLayout);
    this->layout()->setAlignment(Qt::AlignTop);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
}

LogItem* LogContainer::_addLine(Serializable &element, snowflake_uid putUnder) {
    
    auto eId = element.id();
    auto found = this->_linesBySerializableId[eId];

    //if non existant
    if(!found) {

        //create line
        found = new LogItem();

        //if no information about descendant, push last
        if(!putUnder) {
            this->layout()->addWidget(found);
        }
        else {
            //add carriage return symbol
            auto cr = new QLabel("☇");
            found->layout()->setAlignment(Qt::AlignLeft);
            found->layout()->addWidget(cr);

            //find position to put the line under
            auto targetPos = this->_linesBySerializableId[putUnder]->positionInLog();
            
            //add at pos
            this->_vLayout->insertWidget(targetPos+1, found);
        }
        
        //set position
        found->setPositionInLog(this->layout()->count()-1);
        
        //add to store
        this->_linesBySerializableId.insert(eId, found);
    }

    return found;
}

LogItem* LogContainer::_getLine(Serializable &element) {
    return this->_getLine(element.id());
}

LogItem* LogContainer::_getLine(snowflake_uid elementId) {
    return this->_linesBySerializableId[elementId];
}

void LogContainer::clearLines() {
    
    //clear UI
    while(auto item = this->layout()->takeAt(0)) {
        delete item->widget();
        this->layout()->removeItem(item);
    }

    //clear storage
    this->_linesBySerializableId.clear();
}