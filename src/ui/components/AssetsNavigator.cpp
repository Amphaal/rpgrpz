#include "AssetsNavigator.h"

AssetsNavigator::AssetsNavigator(QWidget * parent) : QTreeWidget(parent), _MIMEDb(new QMimeDatabase) {
    this->setHeaderHidden(true);
    this->setDragDropMode(QAbstractItemView::DragDrop);
}


Qt::DropActions AssetsNavigator::supportedDropActions() const
{
    return (Qt::MoveAction | Qt::CopyAction);
}

void AssetsNavigator::dropEvent(QDropEvent *event) {
    auto i = true;
}

void AssetsNavigator::dragMoveEvent(QDragMoveEvent * event) {
    event->acceptProposedAction();
}

void AssetsNavigator::dragEnterEvent(QDragEnterEvent *event) {

    //if dragged from OS
    if (event->mimeData()->hasUrls()) {
        
        for(auto url : event->mimeData()->urls()) {
            
            //if is not local file
            if(!url.isLocalFile()) return;
            
            //if mime of url is image
            auto mimeOfFile = this->_MIMEDb->mimeTypeForUrl(url).name();
            auto pet = mimeOfFile.toStdString();
            if(!mimeOfFile.startsWith("image")) return;
        }

        //if no error, accept
        event->acceptProposedAction();
    
    } else if(event->source()) {

        //if has a widget attached, OK
        event->acceptProposedAction();

    }

}