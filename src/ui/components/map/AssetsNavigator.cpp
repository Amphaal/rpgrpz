#include "AssetsNavigator.h"

AssetsNavigator::AssetsNavigator(QWidget * parent) : QTreeWidget(parent), _MIMEDb(new QMimeDatabase) {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setColumnCount(2);
    this->setHeaderHidden(true);
    this->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    this->setSelectionMode(QAbstractItemView::MultiSelection);

    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}


Qt::DropActions AssetsNavigator::supportedDropActions() const
{
    return (
        Qt::DropAction::MoveAction 
        | Qt::DropAction::CopyAction
    );
}

void AssetsNavigator::dropEvent(QDropEvent *event) {
    // event->setDropAction(Qt::DropAction::MoveAction);
    // event->accept();

    QTreeWidget::dropEvent(event);
}

void AssetsNavigator::dragMoveEvent(QDragMoveEvent * event) {
    // event->acceptProposedAction();
    //QTreeWidget::dragMoveEvent(event);

    QTreeWidget::dragMoveEvent(event);
}

void AssetsNavigator::dragEnterEvent(QDragEnterEvent *event) {

    QTreeWidget::dragEnterEvent(event);

    // //if dragged from OS
    // if (event->mimeData()->hasUrls()) {
        
    //     for(auto url : event->mimeData()->urls()) {
            
    //         //if is not local file
    //         if(!url.isLocalFile()) return;
            
    //         //if mime of url is image
    //         auto mimeOfFile = this->_MIMEDb->mimeTypeForUrl(url).name();
    //         auto pet = mimeOfFile.toStdString();
    //         if(!mimeOfFile.startsWith("image")) return;
    //     }

    //     //if no error, accept
    //     event->acceptProposedAction();
    
    // } else if(event->source()) {

    //     //if has a widget attached, OK
    //     event->acceptProposedAction();

    // }

}