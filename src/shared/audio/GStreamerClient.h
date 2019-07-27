#pragma once

#include <gstreamer-1.0/gst/gst.h>
#include <gstreamer-1.0/gst/gstmessage.h>

#include <gstreamer-1.0/gst/controller/gstinterpolationcontrolsource.h>
#include <gstreamer-1.0/gst/controller/gstdirectcontrolbinding.h>

#include "src/helpers/_appContext.h"

#include <QtCore/QObject>
#include <QtCore/QDir>

#include <QTimer>
#include <QtMath>

class GStreamerClient : public QObject {

    Q_OBJECT
    
    public:
        GStreamerClient(QObject* parent = nullptr);
        ~GStreamerClient();
        

        void useSource(QString uri);
        void setVolume(double volume);
        void play();
        void pause();
        void stop();
        void seek(int seekPos);

    //pseudo private
        GstElement* _bin = nullptr;
        GstBus* _bus = nullptr;

        void _unrefPipeline();
        void _changeBinState(const GstState &state);
        void _requestPosition();

        QTimer* _elapsedTimer = nullptr;
        bool _downloadBufferOK = false;

    signals:
        void positionChanged(int pos);
        void streamEnded();
        void streamError();
        void playStateChanged(bool isPlaying);
        void bufferingPercentChanged(int bufferPrc);

    protected:
        void _initGst();
};