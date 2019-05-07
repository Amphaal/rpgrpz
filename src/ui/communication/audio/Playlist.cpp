#include "Playlist.h"

Playlist::Playlist(QWidget* parent) : QListWidget(parent) {

    // auto a = YoutubeVideo::fromUrl("https://www.youtube.com/watch?v=9bZkp7q19f0");
    // a->fetchVideoInfos();

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
                    
                    //strip file:/// before
                    auto cleanUrl = url.toString(QFlags<QUrl::UrlFormattingOption>(QUrl::RemoveScheme | QUrl::StripTrailingSlash));
                    cleanUrl = cleanUrl.mid(3);
                    this->_buildItemsFromUri(cleanUrl, link.first);

                }
                break;
            case PlaylistItem::LinkType::YoutubePlaylist: {
                    
                    //handler...
                    auto _iterateVideoIds = [&, link](QList<QString> list) {
                        for(auto &id : list) {
                            auto url = YoutubeVideo::urlFromVideoId(id);
                            this->_buildItemsFromUri(url, link.first);
                        }
                    };

                    //fetch videos
                    YoutubeVideo::fromPlaylistUrl(url.toString()).then(_iterateVideoIds);

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
        auto a = new QListWidgetItem();
        auto plItem = new PlaylistItem(type, uri);
        
        //define inner data
        a->setData(
            Qt::UserRole, 
            QVariant::fromValue(static_cast<void*>(plItem))
        );
        
        //define default title
        auto _defineTitle = [a, plItem](QString title) {
            plItem->setTitle(title);
            //auto test = static_cast<PlaylistItem*>(variant.value<void*>());
            a->setText(title);
        };
        _defineTitle(uri); //temporary title

        //conditionnal additionnal informations fetchers
        switch(type) {
            case PlaylistItem::LinkType::ServerAudio:
                AudioFilesHelper::getTitleOfFile(uri).then(_defineTitle);
                break;
            case PlaylistItem::LinkType::YoutubePlaylist:
            case PlaylistItem::LinkType::YoutubeVideo: {
                    auto ytV = YoutubeVideo::fromUrl(uri);
                    ytV->fetchVideoInfos().then([&]() {
                        _defineTitle(ytV->getTitle());
                    });
                }
                
                break;
        }
    
    this->addItem(a);
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
    auto uri = this->_runningLink->text();
    emit playRequested(uri);
}

void Playlist::_onItemDoubleClicked(QListWidgetItem * item) {
    this->_runningLink = item;
    this->_playLink();
}