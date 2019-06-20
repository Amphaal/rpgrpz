#include "Playlist.h"

Playlist::Playlist(QWidget* parent) : QListWidget(parent) {

    //self
    this->setAcceptDrops(true);
    this->setSelectionMode(QAbstractItemView::SingleSelection);

    QObject::connect(
        this, &QListWidget::itemDoubleClicked,
        this, &Playlist::_onItemDoubleClicked
    );
}

Qt::DropActions Playlist::supportedDropActions() const
{
    return (
        Qt::DropAction::MoveAction 
        | Qt::DropAction::CopyAction
    );
}

void Playlist::dragEnterEvent(QDragEnterEvent *event) {

    QWidget::dragEnterEvent(event);

    //if dragged from OS
    if (event->mimeData()->hasUrls()) {
        
        //create list of handled urls
        auto handledUrlsCount = this->_tempHashDnDFromUrlList(event->mimeData()->urls());

        //if there is a single handledUrls, continue
        if(handledUrlsCount) {
            event->setDropAction(Qt::DropAction::MoveAction);
            event->acceptProposedAction();
        }
    
    }

}

int Playlist::_tempHashDnDFromUrlList(QList<QUrl> &list) {
    
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
            this->_tempDnD.append(QPair(PlaylistItem::LinkType::ServerAudio, url));
        
        //from youtube
        } else if (url.host().contains("youtu")) {

            //check if video or playlist link
            auto query = QUrlQuery(url);
            
            //playlist
            if(query.hasQueryItem("list")) {

                //add...
                this->_tempDnD.append(QPair(PlaylistItem::LinkType::YoutubePlaylist, url));
                
            //video
            } else if(query.hasQueryItem("v")) {
                
                //add...
                this->_tempDnD.append(QPair(PlaylistItem::LinkType::YoutubeVideo, url));

            //unhandled format
            } else {
                continue;
            }
        }
            
    }

    //return valid urls count
    return this->_tempDnD.size();
}

void Playlist::dragMoveEvent(QDragMoveEvent * event) {
    event->accept();
}


void Playlist::dropEvent(QDropEvent *event) {
    
    QListWidget::dropEvent(event);

    //for each link registered
    for(auto &link : this->_tempDnD) {
        
        //prepare
        auto url = link.second;

        //defines behavior depending on tag
        switch(link.first) {
            case PlaylistItem::LinkType::ServerAudio: {
                    this->_buildItemsFromUri(url.toLocalFile(), link.first);
                }
                break;
            case PlaylistItem::LinkType::YoutubePlaylist: {
                    
                    //handler...
                    auto _iterateVideoIds = [&, link](const QList<YoutubeVideoMetadata*> &mvideoList) {
                        for(auto mvideo : mvideoList) {
                            this->_buildItemsFromUri(mvideo->url(), PlaylistItem::LinkType::YoutubeVideo);
                        }
                    };

                    //fetch videos
                    YoutubeHelper::fromPlaylistUrl(url.toString()).then(_iterateVideoIds);

                }
                break;
            case PlaylistItem::LinkType::YoutubeVideo:
                this->_buildItemsFromUri(url.toString(), link.first);
                break;
        }

    }

    //clear temp content
    this->_tempDnD.clear();

}

void Playlist::_buildItemsFromUri(QString uri, const PlaylistItem::LinkType &type) {
        
    //prepare item
    auto playlistItem = new QListWidgetItem(uri);
    auto data = new PlaylistItem(type, uri);
    
    //define inner data
    auto c_data_pointer = (long long)data;
    auto c_data_pointer_variant = QVariant::fromValue(c_data_pointer);
    playlistItem->setData(Qt::UserRole, c_data_pointer_variant);

    //update text from playlist update
    QObject::connect(
        data, &PlaylistItem::titleChanged,
        [playlistItem](const QString &title) {
            playlistItem->setText(title);
        }
    );

    this->addItem(playlistItem);
}


void Playlist::playNext() {
    if(this->_defaultPlay()) return; //if default play is used, do nothing

    //find next elem
    auto selected_index = this->indexFromItem(this->_runningLink);
    auto next_index = selected_index.siblingAtRow(selected_index.row() + 1);
    
    //if next elem is ok
    if(next_index.isValid()) {
        this->_runningLink = this->itemFromIndex(next_index);
        this->_playLink();
    } else {
        this->_runningLink = nullptr;
        this->_defaultPlay();
    };
}

void Playlist::playPrevious() {
    if(this->_defaultPlay()) return; //if default play is used, do nothing

    //find previous elem
    auto selected_index = this->indexFromItem(this->_runningLink);
    auto prev_index = selected_index.siblingAtRow(selected_index.row() - 1);
    
    //if next elem is ok
    if(prev_index.isValid()) {
        this->_runningLink = this->itemFromIndex(prev_index);
        this->_playLink();
    } else {
        this->_runningLink = nullptr;
        this->_defaultPlay();
    };

}

bool Playlist::_defaultPlay() {
    if(!this->count()) return false; //if no items, do nothing
    if(this->_runningLink) return false; //if running link set, do nothing

    //select the first item, and play
    this->_runningLink = this->itemFromIndex(this->model()->index(0,0));
    this->_playLink();
    return true;
}

void Playlist::_playLink() {
    auto data_variant = this->_runningLink->data(Qt::UserRole);
    auto asLong = data_variant.toLongLong();
    auto p_data = (PlaylistItem*)asLong;
    emit playRequested((void*)p_data);
}

void Playlist::_onItemDoubleClicked(QListWidgetItem * item) {
    this->_runningLink = item;
    this->_playLink();
}