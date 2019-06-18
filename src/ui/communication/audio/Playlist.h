#include <QListWidget>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QMimeData>

#include <QHash>
#include <QPair>

#include <QUrlQuery>

#include "PlaylistItem.h"

class Playlist : public QListWidget {

    Q_OBJECT

    public:
        Playlist(QWidget* parent = nullptr);

        void playNext();
        void playPrevious();
    
    signals:
        void playRequested(void* data);

    private:
        void _onItemDoubleClicked(QListWidgetItem * item);
        QListWidgetItem* _runningLink = nullptr;

        //drag and drop
            QMimeDatabase _MIMEDb;
            Qt::DropActions supportedDropActions() const override;
            void dragEnterEvent(QDragEnterEvent *event) override;
            void dragMoveEvent(QDragMoveEvent * event) override;
            void dropEvent(QDropEvent *event) override;

            //d&d temp
            QList<QPair<PlaylistItem::LinkType, QUrl>> _tempDnD;
            int _tempHashDnDFromUrlList(QList<QUrl> &list);
            void _buildItemsFromUri(QString uri, const PlaylistItem::LinkType &type);
    

        bool _defaultPlay();
        void _playLink();
};
