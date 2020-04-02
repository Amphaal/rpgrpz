// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include <gst/gst.h>
#include <gst/gstmessage.h>

#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gst/controller/gstdirectcontrolbinding.h>

#include "src/helpers/_appContext.h"

#include <QObject>
#include <QDir>

#include <QTimer>
#include <QTimeLine>
#include <QtMath>

#include <QMutexLocker>

struct BufferedSeek {
    QDateTime ts;
    gint64 posInNano = 0;
};

extern bool gst_client_bus_cb(GstBus *bus, GstMessage *msg, void* data);

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
        void seek(qint64 seekPosInMsecs);
    
    //pseudo private
        GstElement* _bin = nullptr;
        GstBus* _bus = nullptr;
        
        void _initPipeline();
        void _unrefPipeline();

        void _changeBinState(const GstState &state);
        void _requestPosition();

        QTimer* _elapsedTimer = nullptr;
        bool _downloadBufferOK = false;

        //seek
        mutable QMutex _m_seek;
        bool _mayQuerySeekRange = true;
        bool _seekRangeUpToDate = false;
        QPair<gint64, gint64> _seekableRange;
        BufferedSeek _seekBuffer;

        void _freeSeekBuffer();

    public slots:
        void stopTimer(const GstMessageType &reason);
        void downloadBufferChanging(int prcProgress);
        bool _seek(gint64 seekInNanoSecs);

    signals:
        void positionChanged(int positionInSecs);
        void streamEnded();
        void streamError();
        void playStateChanged(bool isPlaying);
        void bufferingPercentChanged(int bufferPrc);

    protected:
        void _initGst();
    
    private:
        //volume
        QTimeLine _volumeTLHelper;
};
