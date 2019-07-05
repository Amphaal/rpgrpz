#include "GStreamerClient.h"

/////////////
/////////////

extern "C" {

    static bool gst_rpgrpz_bus_cb(GstBus *bus, GstMessage *msg, void* data) {

        //cast cli
        GStreamerClient* cli = static_cast<GStreamerClient*>(data);

        //on message type
        auto type = GST_MESSAGE_TYPE (msg);
        // auto test = gst_message_type_get_name (type);
        // g_print ("%s\n", test);

        //switch
        switch (type) {
            case GST_MESSAGE_ERROR: {
                GError *err;
                gchar *debug_info;
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
            }
            break;

            case GST_MESSAGE_EOS: {
                g_print ("End-Of-Stream reached.\n");
            }
            break;

            case GST_MESSAGE_BUFFERING: {
                
                gint percent;
                gst_message_parse_buffering(msg, &percent);
                cli->_downloadBufferOK = percent == 100;
                emit cli->bufferingPercentChanged(percent);

            }
            break;
            
            default:
                break;
        }

        //keep receiving messages
        return true;

    }
}

////////////
////////////

GStreamerClient::GStreamerClient(QObject* parent) : QObject(parent), _elapsedTimer(new QTimer) {
    
    this->_initGst();
    
    //timer
    QObject::connect(
        this->_elapsedTimer, &QTimer::timeout, 
        this, &GStreamerClient::_requestPosition
    );

    //inst elements
    this->_bin = gst_element_factory_make("playbin", "bin");
    if(!this->_bin) {
    	this->_unrefPipeline();
        throw std::runtime_error("Unable to init playbin");
    }
    
    this->setVolume(
        AppContext::settings()->audioVolume()
    );
    
    //define bus and callbacks
    this->_bus = gst_pipeline_get_bus(GST_PIPELINE(this->_bin));
    if(!this->_bus) {
    	this->_unrefPipeline();
    	throw std::runtime_error("Unable to init bus for playbin");
    }

    //event handler
    gst_bus_set_sync_handler(this->_bus, gst_bus_sync_signal_handler, this, NULL);
    g_object_connect (this->_bus, "signal::sync-message", G_CALLBACK(gst_rpgrpz_bus_cb), this, NULL);

}

GStreamerClient::~GStreamerClient() {
    this->_unrefPipeline();
}

void GStreamerClient::useSource(QString uri) {
    
    //stop
    this->stop();

    //set new source
    g_object_set(G_OBJECT(this->_bin), "uri", uri.toStdString().c_str(), NULL);

    //play
    this->play();
    
}

void GStreamerClient::seek(int seekPos) {
    auto nano_seekPos = GST_SECOND * seekPos;
    auto worked = gst_element_seek_simple(
        this->_bin, 
        GST_FORMAT_TIME, 
        (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 
        nano_seekPos
    );
}

void GStreamerClient::setVolume(double volume) {
    //convert and limit
    volume = volume / 100;
    if (volume > 1) volume = 1;

    //set new volume
    g_object_set(G_OBJECT(this->_bin), "volume", volume, NULL);

}

void GStreamerClient::play() {
    this->_changeBinState(GST_STATE_PLAYING);
    this->_elapsedTimer->start(1000);
    this->_requestPosition();
}

void GStreamerClient::pause() {
    this->_changeBinState(GST_STATE_PAUSED);
    this->_elapsedTimer->stop();
    this->_requestPosition();
}

void GStreamerClient::stop() {
    this->_changeBinState(GST_STATE_READY);
    this->_elapsedTimer->stop();
    emit positionChanged(0);
}

void GStreamerClient::_unrefPipeline() {
    if(this->_bus) gst_object_unref(GST_OBJECT(this->_bus));
    if(this->_bin) {
        gst_element_set_state(this->_bin, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(this->_bin));
    }
}

void GStreamerClient::_changeBinState(const GstState &state) {
    auto ret = gst_element_set_state(this->_bin, state);
    
    if (ret == GST_STATE_CHANGE_FAILURE) {
        this->_unrefPipeline();
        throw std::runtime_error("Unable to set the pipeline to the playing state.\n");
    }

}

void GStreamerClient::_requestPosition() {   
    
    gint64 pos;
    gst_element_query_position (this->_bin, GST_FORMAT_TIME, &pos);
    if(pos == -1) return;

    int posSec = (int)(pos / GST_SECOND);

    emit positionChanged(posSec);
}

void GStreamerClient::_initGst() {   

    //add plugin dir detection
    auto td = QCoreApplication::applicationDirPath().toStdString() + "/gst-plugins";
    qputenv("GST_PLUGIN_PATH", td.c_str());

    //gio module dir
    auto gio_ = QCoreApplication::applicationDirPath().toStdString() + "/gio";
    qputenv("GIO_MODULE_DIR", gio_.c_str());

    //setup
    GError* err = NULL;
    gst_init_check(NULL, NULL, &err);
    if(err) {
        throw std::runtime_error("Could not instanciate GStreamer !");
    }
}
