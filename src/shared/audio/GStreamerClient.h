#include <gstreamer-1.0/gst/gst.h>
#include <gstreamer-1.0/gst/gstmessage.h>

#include <gstreamer-1.0/gst/controller/gstinterpolationcontrolsource.h>
#include <gstreamer-1.0/gst/controller/gstdirectcontrolbinding.h>

#include "src/helpers/_appContext.h"

#include <QtCore/QObject>
#include <QtCore/QDir>

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
        void positionChanged(int pos);
        void endOfStream();
        void downloadedKbps(double kbps);

    private:
        GstElement* _bin = nullptr;
        GstBus* _bus = nullptr;

        void _unrefPipeline();
        void _changeBinState(const GstState &state);

    protected:
        void _initGst();
};
