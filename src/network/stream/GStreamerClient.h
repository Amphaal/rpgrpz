#include <gst/gst.h>
#include <gst/gstmessage.h>

#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gst/controller/gstdirectcontrolbinding.h>

#include <QObject>
#include <QDir>

class GStreamerClient : public QObject {

    Q_OBJECT
    
    public:
        struct CB;
        GStreamerClient(QObject* parent = nullptr);
        ~GStreamerClient();

        void useSource(QString uri);
        void setVolume(double volume);
        void play();
        void pause();
        void stop();

    signals:
        void playingState(int pos);
        void endOfStream();
        void downloadedKbps(double kbps);

    private:
        GstElement* _bin = NULL;
        GstElement* _sourceFile = NULL;
        GstElement* _output = NULL;

        GstControlSource* _volumeControl = NULL;
        GstControlSource* _faderControl = NULL;

        void _unrefPipeline();
        void _changeBinState(const GstState &state);

    protected:
        void _initGst();
};