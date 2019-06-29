#include "GStreamerClient.h"

/////////////
/////////////

struct GStreamerClient::CB {

  static void _onEndOfStream(GstBus *bus, GstMessage *msg, void* data) {
    GStreamerClient* cli = static_cast<GStreamerClient*>(data);
    emit cli->endOfStream();
  }

};

extern "C" {
  static void eos_cb(GstBus *bus, GstMessage *msg, void* data) {
    GStreamerClient::CB::_onEndOfStream(bus, msg, data);
  }
}

////////////
////////////

GStreamerClient::GStreamerClient(QObject* parent) : QObject(parent) {
    
    this->_initGst();

    //inst elements
    this->_bin = gst_element_factory_make ("playbin", "playbin");
    
    this->setVolume(
        AppContext::settings()->audioVolume()
    );
    
    //define bus and callbacks
    this->_bus = gst_element_get_bus(this->_bin);
    gst_bus_add_signal_watch(this->_bus);
    g_signal_connect(G_OBJECT(this->_bus), "message::eos", (GCallback)eos_cb, this);

}

GStreamerClient::~GStreamerClient() {
    this->_unrefPipeline();
}

void GStreamerClient::useSource(QString uri) {
    
    //stop
    this->stop();

    //set new source
    g_object_set(this->_bin, "uri", uri.toStdString().c_str(), NULL);

    //play
    this->play();
    
}

void GStreamerClient::setVolume(double volume) {
    //convert and limit
    volume = volume / 100;
    if (volume > 1) volume = 1;

    //set new volume
    g_object_set(this->_bin, "volume", volume, NULL);

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
    if(!this->_bin) return;
    gst_object_unref(this->_bus);
    gst_element_set_state (this->_bin, GST_STATE_NULL);
    gst_object_unref(this->_bin);
}

void GStreamerClient::_changeBinState(const GstState &state) {
    auto ret = gst_element_set_state(this->_bin, state);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        this->_unrefPipeline();
        return;
    }
}

void GStreamerClient::_initGst() {
    //plugin dir
    auto td = QDir::toNativeSeparators(QDir::currentPath()).toStdString() + "\\gst-plugins";
    qputenv("GST_PLUGIN_PATH", td.c_str());
    
    //setup
    GError* err = NULL;
    gst_init_check(NULL, NULL, &err);
    if(err) {
        throw std::runtime_error("Could not instanciate GStreamer !");
        return;
    }
}