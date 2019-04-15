#pragma once

#include <QHash>
#include <QPair>

#include <QGroupBox>
#include <QVBoxLayout>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QMimeData>

#include <QListWidget>

#include <QUrlQuery>

#include "src/ui/others/ClientBindable.hpp"

#include "PlaylistToolbar.hpp"

class PlaylistController : public QGroupBox, public ClientBindable {
    
    public:
        enum LinkType { YoutubePlaylist, YoutubeVideo, ServerAudio };

        PlaylistController(QWidget * parent = nullptr);

        void bindToRPZClient(RPZClient * cc) override;

    private:

        //drag and drop
            QMimeDatabase _MIMEDb;
            void dropEvent(QDropEvent *event) override;
            void dragEnterEvent(QDragEnterEvent *event) override;

            //d&d temp
            QList<QPair<PlaylistController::LinkType, QUrl>> _tempDnD;
            int tempHashDnDFromUrlList(QList<QUrl> &list);

        QListWidget* _list;
        PlaylistToolbar* _toolbar;
        
};