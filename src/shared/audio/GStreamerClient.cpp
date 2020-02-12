#include "GStreamerClient.h"

GStreamerClient::GStreamerClient(QObject* parent) : QObject(parent), _elapsedTimer(new QTimer) {
    
    //define volume helper
    this->_volumeTLHelper.setDuration(100);
    this->_volumeTLHelper.setDirection(QTimeLine::Direction::Backward);
    this->_volumeTLHelper.setEasingCurve(QEasingCurve::InCubic);

    //inst
    this->_initGst();
    
    //timer
    QObject::connect(
        this->_elapsedTimer, &QTimer::timeout, 
        this, &GStreamerClient::_requestPosition
    );

    this->_initPipeline();

}

void GStreamerClient::_initGst() {   

    //add plugin dir detection
    auto td = QCoreApplication::applicationDirPath() + "/gst-plugins";
    qputenv("GST_PLUGIN_PATH", qUtf8Printable(td));

    //gio module dir
    auto gio_ = QCoreApplication::applicationDirPath() + "/gio";
    qputenv("GIO_MODULE_DIR", qUtf8Printable(gio_));

    //setup
    GError* err = NULL;
    gst_init_check(NULL, NULL, &err);
    if(err) {
        throw std::runtime_error("Could not instanciate GStreamer !");
    }
}

GStreamerClient::~GStreamerClient() {
    this->_unrefPipeline();
}

void GStreamerClient::_initPipeline() {

    //inst elements
    this->_bin = gst_element_factory_make("playbin", "bin");
    if(!this->_bin) {
    	this->_unrefPipeline();
        throw std::runtime_error("Unable to init playbin");
    }
    g_object_set(this->_bin, "async-handling", true, nullptr);

    //force auto sink (required!)
    auto sink = gst_element_factory_make("autoaudiosink", "sink");
    g_object_set(this->_bin, "audio-sink", sink, nullptr);

    //init bin
    this->setVolume(AppContext::settings()->audioVolume());
    
    //define bus and callbacks
    this->_bus = gst_pipeline_get_bus(GST_PIPELINE(this->_bin));
    if(!this->_bus) {
    	this->_unrefPipeline();
    	throw std::runtime_error("Unable to init bus for playbin");
    }

    //event handler
    gst_bus_set_sync_handler(this->_bus, gst_bus_sync_signal_handler, this, nullptr);
    g_object_connect(this->_bus, "signal::sync-message", G_CALLBACK(gst_client_bus_cb), this, nullptr);

}

void GStreamerClient::_unrefPipeline() {
    
    if(this->_bus) {
        gst_object_unref(GST_OBJECT(this->_bus));
        this->_bus = nullptr;
    }

    if(this->_bin) {
        gst_element_set_state(this->_bin, GST_STATE_NULL);
        gst_object_unref(this->_bin);
        this->_bin = nullptr;
    }

}

void GStreamerClient::_changeBinState(const GstState &state) {
    
    if(!this->_bin) return;

    auto ret = gst_element_set_state(this->_bin, state);
    
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "Gstreamer : cannot change bin state !";
    };

}

void GStreamerClient::useSource(QString uri) {

    //set new source
    g_object_set(G_OBJECT(this->_bin), "uri", qUtf8Printable(uri), nullptr);

    qDebug() << "Trying to play : " << qUtf8Printable(uri);

    {
        QMutexLocker l(&this->_m_seek);

        //allow requesting seek range
        this->_mayQuerySeekRange = true;
        this->_seekRangeUpToDate = false;
    }

}

void GStreamerClient::play() {
    
    this->_changeBinState(GST_STATE_PLAYING);
    this->_elapsedTimer->start(500);
    this->_requestPosition();
    emit playStateChanged(true);

    //debug graph (use https://dreampuf.github.io/GraphvizOnline to read it)
    //GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(this->_bin), GST_DEBUG_GRAPH_SHOW_ALL, "gst-pipeline");

}

void GStreamerClient::pause() {
    
    this->_changeBinState(GST_STATE_PAUSED);
    this->_elapsedTimer->stop();
    
    this->_requestPosition();
    emit playStateChanged(false);

}

void GStreamerClient::stop() {
    
    this->_changeBinState(GST_STATE_NULL);
    this->_elapsedTimer->stop();
    
    emit positionChanged(-1);
    emit playStateChanged(false);

}


void GStreamerClient::setVolume(double volume) {
    
    //cap
    if (volume > 100) volume = 100;

    //apply curve
    volume = this->_volumeTLHelper.valueForTime((int)volume);

    //set new volume
    g_object_set(G_OBJECT(this->_bin), "volume", volume, nullptr);

}

void GStreamerClient::seek(qint64 seekPosInMsecs) {
    
    gint64 nano_seekPos = GST_MSECOND * (gint64)seekPosInMsecs;
    
    //buffer
    this->_m_seek.lock();
    if(!this->_seekRangeUpToDate) {
        
        BufferedSeek buf;
        buf.posInNano = nano_seekPos;

        if(this->_bin->current_state == GST_STATE_PLAYING || this->_bin->target_state == GST_STATE_PLAYING) {
            buf.ts = QDateTime::currentDateTime();
        }

        this->_seekBuffer = buf;
        this->_m_seek.unlock();
    } 
    
    //seek
    else {
        this->_m_seek.unlock();
        this->_seek(nano_seekPos);
    }
    
}

bool GStreamerClient::_seek(gint64 seekInNanoSecs) {

    //check if can seek
    auto currentState = this->_bin->current_state;
    auto canSeek = currentState == GST_STATE_PLAYING || currentState == GST_STATE_PAUSED;
    if(!canSeek) return false;

    //check if seek is too low, prevent that from happening since it may freeze playing
    if(seekInNanoSecs < 1 * GST_SECOND) return false;

    // g_print ("Seeking to... %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (seekInNanoSecs));

    //seek
    auto result = gst_element_seek_simple(
        this->_bin, 
        GST_FORMAT_TIME,
        GstSeekFlags(
            GST_SEEK_FLAG_FLUSH
            // | GST_SEEK_FLAG_KEY_UNIT
            // | GST_SEEK_FLAG_SNAP_NEAREST
            | GST_SEEK_FLAG_ACCURATE
        ),
        seekInNanoSecs
    );

    return result;

}

void GStreamerClient::_freeSeekBuffer() {
    
    QMutexLocker l(&this->_m_seek);

    //extract buffer values
    auto bufpos = this->_seekBuffer.posInNano;
    auto bufTs = this->_seekBuffer.ts;

    //check if a seek buffer is waiting
    if(bufpos != 0) {

        //reset buffer
        BufferedSeek buf;
        this->_seekBuffer = buf;
        
        //update bufpos is buffer was registered while playing was requested
        if(!bufTs.isNull()) {
            auto diffMsec = bufTs.msecsTo(QDateTime::currentDateTime());
            auto diffNsec = GST_MSECOND * diffMsec;
            bufpos = bufpos + diffNsec;
        }

        //check if Out of Bound 
        auto oobSeekOperation = 
            this->_seekableRange.first > bufpos ||
            this->_seekableRange.second < bufpos;

        if(oobSeekOperation) return;

        l.unlock();

        //async seek, as this function could have been called from and inner GST thread
        QMetaObject::invokeMethod(this, "_seek", 
            Q_ARG(gint64, bufpos)
        );

    }
}

void GStreamerClient::_requestPosition() {   
    
    gint64 pos;
    gst_element_query_position (this->_bin, GST_FORMAT_TIME, &pos);
    if(pos == -1) return;

    auto posSec = qCeil((double)pos / GST_SECOND);

    emit positionChanged(posSec);

}

void GStreamerClient::downloadBufferChanging(int prcProgress) {
    this->_downloadBufferOK = prcProgress == 100;
    // qDebug() << "Download buffer..." << prcProgress;
    emit bufferingPercentChanged(prcProgress);
}


void GStreamerClient::stopTimer(const GstMessageType &reason) {
    
    this->_elapsedTimer->stop();

    switch(reason) {
        
        case GST_MESSAGE_ERROR:
            qWarning() << "Gstreamer : Error from Stream";
            emit streamError();
        break;

        case GST_MESSAGE_EOS:
            emit streamEnded();
        break;

        default:
        break;

    }

}
/////////////
/////////////

bool gst_client_bus_cb(GstBus *bus, GstMessage *msg, void* data) {

    //cast cli
    GStreamerClient* cli = static_cast<GStreamerClient*>(data);

    //switch
    switch (auto type = GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            
            GError *err;
            gchar *debug_info;
            gst_message_parse_error (msg, &err, &debug_info);
            g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
            g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error (&err);
            g_free (debug_info);
            
            QMetaObject::invokeMethod(cli, "stopTimer", 
                Q_ARG(GstMessageType, type)
            );

        }
        break;

        case GST_MESSAGE_STATE_CHANGED: {
            
            //get update inner state
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            
            //if playbin is not the target, skip
            if(GST_MESSAGE_SRC(msg) != GST_OBJECT(cli->_bin)) break;

            QMutexLocker l(&cli->_m_seek);

            //check if in play state and refresh of seekable must be done
            auto isPlaying = (new_state == GST_STATE_PLAYING);
            auto shouldQuery = (cli->_mayQuerySeekRange && !cli->_seekRangeUpToDate && isPlaying);
            if(!isPlaying) cli->_seekRangeUpToDate = false;
            if(!shouldQuery) break;

            //create a new query to request seek ranges
            auto query = gst_query_new_seeking(GST_FORMAT_TIME);
            
            //request values from query
            if (gst_element_query (cli->_bin, query)) {
                
                //parse values
                gint64 start, end;
                gboolean seek_enabled;
                gst_query_parse_seeking (query, NULL, &seek_enabled, &start, &end);

                //if can seek
                if (seek_enabled) {
                    
                    //update seek range
                    cli->_seekableRange = QPair<gint64, gint64>(start, end);
                    cli->_seekRangeUpToDate = true;
                    
                    //free seek buffer if any
                    l.unlock();
                    cli->_freeSeekBuffer();

                } 
                
                //if cant, prevent further querying until source changed
                else {
                    cli->_mayQuerySeekRange = false;
                    cli->_seekableRange = QPair<gint64, gint64>();
                    g_print ("Seeking is DISABLED for this stream.\n");
                }

            } 
            
            //on error while querying
            else {
                cli->_seekableRange = QPair<gint64, gint64>();
                g_printerr ("Seeking query failed.");
            }

            //unref query
            gst_query_unref (query);
            
        }
        break;

        case GST_MESSAGE_EOS: {
            QMetaObject::invokeMethod(cli, "stopTimer", 
                Q_ARG(GstMessageType, type)
            );
        }
        break;

        case GST_MESSAGE_BUFFERING: {
            
            gint percent;
            gst_message_parse_buffering(msg, &percent);
            
            QMetaObject::invokeMethod(cli, "downloadBufferChanging", 
                Q_ARG(int, percent)
            );

        }
        break;
        
        default:
            break;
    }

    //keep receiving messages
    return true;

}

////////////
////////////