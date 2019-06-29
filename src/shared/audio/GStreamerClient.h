#include <gst/gst.h>
#include <gst/gstmessage.h>

#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gst/controller/gstdirectcontrolbinding.h>

#include "src/helpers/_appContext.h"

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