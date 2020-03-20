#include "Playlist.h"

Playlist::Playlist(QWidget* parent) : QListWidget(parent),
    _ytIconGrey(new QIcon(QStringLiteral(u":/icons/app/audio/youtubeGrey.png"))),
    _ytIcon(new QIcon(QStringLiteral(u":/icons/app/audio/youtube.png"))),
    _ytIconErr(new QIcon(QStringLiteral(u":/icons/app/audio/youtubeError.png"))) {

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
        auto urls = event->mimeData()->urls();
        auto handledUrlsCount = this->_tempHashDnDFromUrlList(urls);

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
    for(const auto &url : list) {
        
        //if is not local file
        if(!url.isValid()) continue;

        //for local files...
        if (url.host().contains(QStringLiteral(u"youtu"))) {

            //check if video or playlist link
            auto query = QUrlQuery(url);
            
            //playlist
            if(query.hasQueryItem("list")) {

                //add...
                this->_tempDnD.append(QPair(YoutubeUrlType::YoutubePlaylist, url));
                
            //video
            } else if(query.hasQueryItem("v")) {
                
                //add...
                this->_tempDnD.append(QPair(YoutubeUrlType::YoutubeVideo, url));

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
    for(const auto &[type, url] : this->_tempDnD) {

        //defines behavior depending on tag
        switch(type) {

            case YoutubeUrlType::YoutubePlaylist: {

                //fetch videos from playlist
                YoutubeHelper::fromPlaylistUrl(url.toString()).then([=](const QList<VideoMetadata*> &mvideoList) {
                    for(const auto mvideo : mvideoList) {
                        this->addYoutubeVideo(mvideo->url());
                    }
                });

            }
            break;

            case YoutubeUrlType::YoutubeVideo: {
                this->addYoutubeVideo(url.toString());
            }
            break;

        }

    }

    //clear temp content
    this->_tempDnD.clear();

}


void Playlist::addYoutubeVideo(const QString &url) {
    
    //metadata definition
    auto data = VideoMetadata::fromVideoUrl(url);
    auto videoId = data->id();

    //handle duplicates
    if(this->_playlistVideoIds.contains(videoId)) {
        QToolTip::showText(this->mapToGlobal(QPoint()), tr("This Youtube video is already in playlist !"));
        delete data;
        return;
    }
    this->_playlistVideoIds.insert(videoId);
    auto pos = QString::number(this->_playlistVideoIds.count()) + ". ";

    //prepare item
    auto playlistItem = new QListWidgetItem(pos + url);   

    //define inner data
    RPZQVariant::setYTVideoMetadata(playlistItem, data);

    //define default icon
    playlistItem->setIcon(*this->_ytIconGrey);

    //update text from playlist update
    QObject::connect(
        data, &VideoMetadata::metadataRefreshed,
        [=]() {

            auto durationStr = StringHelper::secondsToTrackDuration(data->duration());

            auto title = QStringLiteral(u"%1 [%2]")
                            .arg(data->title())
                            .arg(durationStr);

            playlistItem->setText(pos + title);
            
            //define active YT icon
            playlistItem->setIcon(*this->_ytIcon);

        }
    );

    QObject::connect(
        data, &VideoMetadata::metadataFetching,
        [=]() {  
            playlistItem->setIcon(*this->_ytIconGrey);
            playlistItem->setText(pos + tr("(Loading metadata...) ") + data->url()); 
        }
    );

    QObject::connect(
        data, &VideoMetadata::streamFailed,
        [=]() {
            //add delay for user ack
            QTimer::singleShot(100, [=]() {
                playlistItem->setIcon(*this->_ytIconErr);
                playlistItem->setText(pos + tr("(Error) ") + data->url()); 
            });  
        }
    );

    this->addItem(playlistItem);
}


void Playlist::playNext() {
    if(this->_defaultPlay()) return; //if default play is used, do nothing

    //find next elem
    auto selected_index = this->indexFromItem(this->_playlistItemToUse);
    auto next_index = selected_index.siblingAtRow(selected_index.row() + 1);
    
    //if next elem is ok
    if(next_index.isValid()) {
        this->_playlistItemToUse = this->itemFromIndex(next_index);
        this->_requestPlay();
    } else {
        this->_playlistItemToUse = nullptr;
        this->_defaultPlay();
    };
}

void Playlist::playPrevious() {
    if(this->_defaultPlay()) return; //if default play is used, do nothing

    //find previous elem
    auto selected_index = this->indexFromItem(this->_playlistItemToUse);
    auto prev_index = selected_index.siblingAtRow(selected_index.row() - 1);
    
    //if next elem is ok
    if(prev_index.isValid()) {
        this->_playlistItemToUse = this->itemFromIndex(prev_index);
        this->_requestPlay();
    } else {
        this->_playlistItemToUse = nullptr;
        this->_defaultPlay();
    };

}

bool Playlist::_defaultPlay() {
    if(!this->count()) return false; //if no items, do nothing
    if(this->_playlistItemToUse) return false; //if running link set, do nothing

    //select the first item, and play
    this->_playlistItemToUse = this->itemFromIndex(this->model()->index(0,0));
    this->_requestPlay();
    return true;
}

void Playlist::_requestPlay() {
    auto currentPlay = this->currentPlay();
    if(currentPlay) emit playRequested(currentPlay);
}

void Playlist::_onItemDoubleClicked(QListWidgetItem * item) {
    this->_playlistItemToUse = item;
    this->_requestPlay();
}

VideoMetadata* Playlist::currentPlay() {
    if(!this->_playlistItemToUse) return nullptr;
    return RPZQVariant::ytVideoMetadata(this->_playlistItemToUse);
}