#include "PlaylistController.h"

PlaylistController::PlaylistController(QWidget * parent) : QGroupBox(parent), 
    _list(new QListWidget(this)), 
    _toolbar(new PlaylistToolbar(this)) {

    //self
    this->setAcceptDrops(true);
    this->setEnabled(false);
    this->setLayout(new QVBoxLayout);

    //groupbox
    this->setTitle("Liste de lecture");
    this->setAlignment(Qt::AlignHCenter);

    //inner list
    this->layout()->addWidget(this->_toolbar);
    this->layout()->addWidget(this->_list);
};


void PlaylistController::bindToRPZClient(RPZClient * cc) {
    ClientBindable::bindToRPZClient(cc);
    this->setEnabled(true);

    QObject::connect(
       cc, &JSONSocket::disconnected,
       [&]() {
           this->setEnabled(false);
       }
    );
}

void PlaylistController::dropEvent(QDropEvent *event) {
    
    QWidget::dropEvent(event);

    //for each link registered
    for(auto &link : this->_tempDnD) {
        
        //defines behavior depending on tag
        switch(link.first) {
            case PlaylistController::ServerAudio:
                break;
            case PlaylistController::YoutubePlaylist:
                break;
            case PlaylistController::YoutubeVideo:
                break;
        }

        //add item de list
        this->_list->addItem(link.second.toString());
    }

    //clear temp content
    this->_tempDnD.clear();

}

int PlaylistController::tempHashDnDFromUrlList(QList<QUrl> &list) {
    
    //clear temp content
    this->_tempDnD.clear();
    
    //iterate through
    for(auto &url : list) {
        
        //if is not local file
        if(!url.isValid()) continue;

        //for local files...
        if(url.isLocalFile()) { 
            
            //only audio local files
            auto mimeOfFile = this->_MIMEDb.mimeTypeForUrl(url).name();
            if(!mimeOfFile.startsWith("audio")) continue;

            //add...
            this->_tempDnD.append(QPair(PlaylistController::ServerAudio, url));
        
        //from youtube
        } else if (url.host().contains("youtu")) {

            //check if video or playlist link
            auto query = QUrlQuery(url);
            
            //playlist
            if(query.hasQueryItem("list")) {

                //add...
                this->_tempDnD.append(QPair(PlaylistController::YoutubePlaylist, url));
                
            //video
            } else if(query.hasQueryItem("v")) {
                
                //add...
                this->_tempDnD.append(QPair(PlaylistController::YoutubeVideo, url));

            //unhandled format
            } else {
                continue;
            }
        }
            
    }

    //return valid urls count
    return this->_tempDnD.size();
}

void PlaylistController::dragEnterEvent(QDragEnterEvent *event) {

    QWidget::dragEnterEvent(event);

    //if dragged from OS
    if (event->mimeData()->hasUrls()) {
        
        //create list of handled urls
        auto handledUrlsCount = this->tempHashDnDFromUrlList(event->mimeData()->urls());

        //if there is a single handledUrls, continue
        if(handledUrlsCount) {
            event->setDropAction(Qt::DropAction::MoveAction);
            event->acceptProposedAction();
        }
    
    }

}