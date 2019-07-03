#include "GStreamerClient.h"

/////////////
/////////////

void GStreamerClient::CB::_handleMessages(GstBus *bus, GstMessage *msg, void* data) {
    GStreamerClient* cli = static_cast<GStreamerClient*>(data);
    auto i = true;
}


extern "C" {
  static void gst_cb(GstBus *bus, GstMessage *msg, void* data) {
    GStreamerClient::CB::_handleMessages(bus, msg, data);
  }
}

////////////
////////////

GStreamerClient::GStreamerClient(QObject* parent) : QObject(parent) {
    
    this->_initGst();

    //inst elements
    this->_bin = gst_element_factory_make ("playbin", "bin");
    if(!this->_bin) {
    	this->_unrefPipeline();
        throw std::runtime_error("Unable to init playbin");
    }
    
    this->setVolume(
        AppContext::settings()->audioVolume()
    );
    
    //define bus and callbacks
    this->_bus = gst_element_get_bus(this->_bin);
    if(!this->_bus) {
    	this->_unrefPipeline();
    	throw std::runtime_error("Unable to init bus for playbin");
    }

    gst_bus_add_watch(this->_bus, (GstBusFunc)gst_cb, this->_bus);

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

void GStreamerClient::setVolume(double volume) {
    //convert and limit
    volume = volume / 100;
    if (volume > 1) volume = 1;

    //set new volume
    g_object_set(G_OBJECT(this->_bin), "volume", volume, NULL);

}

void GStreamerClient::play() {
    //TODO fadein
    this->_changeBinState(GST_STATE_PLAYING);
}

void GStreamerClient::pause() {
    //TODO fadeout
    this->_changeBinState(GST_STATE_PAUSED);
}

void GStreamerClient::stop() {
    //TODO fadeout
    this->_changeBinState(GST_STATE_READY);
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
