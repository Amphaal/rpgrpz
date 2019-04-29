#include <QListWidget>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QMimeData>

#include <QHash>
#include <QPair>

#include <QUrlQuery>

#include "src/helpers/network/_YoutubeHelper.hpp"
// #include "src/helpers/network/_AudioFilesHelper.hpp"

class Playlist : public QListWidget {

    Q_OBJECT

    public:
        Playlist(QWidget* parent = nullptr);

        enum LinkType { YoutubePlaylist, YoutubeVideo, ServerAudio };

        void playNext();
        void playPrevious();
    
    signals:
        void playRequested(const QString &uri);

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
            QList<QPair<LinkType, QUrl>> _tempDnD;
            int _tempHashDnDFromUrlList(QList<QUrl> &list);
            void _buildItemsFromUri(QString uri, const LinkType &type);
    

        bool _defaultPlay();
        void _playLink();
};