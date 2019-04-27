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
    this->_bin = gst_pipeline_new("bin");
    this->_sourceFile = gst_element_factory_make("filesrc", NULL);
    auto volumeFilter = gst_element_factory_make("volume", NULL);
    auto faderFilter = gst_element_factory_make("volume", NULL);
    this->_output = gst_element_factory_make("autoaudiosink", NULL);

    auto parser = gst_element_factory_make("mpegaudioparse", NULL);
    auto decoder = gst_element_factory_make("mpg123audiodec", NULL);
    auto converter = gst_element_factory_make("audioconvert", NULL);
    auto resampler =  gst_element_factory_make("audioresample", NULL);

    auto allSet = this->_bin && 
    this->_sourceFile && 
    volumeFilter && 
    faderFilter && 
    this->_output && 
    parser && decoder 
    && converter && resampler;
    if(!allSet) {
         throw new std::exception("gst plugins missing !");
         return;
    }

    //Build the pipeline
    gst_bin_add_many(GST_BIN(this->_bin), 
        this->_sourceFile, 
        parser,
        decoder,
        converter,
        resampler,
        faderFilter,
        volumeFilter,
        this->_output, 
        NULL);
    
    //link it
    if (gst_element_link_many(
        this->_sourceFile,
        parser,
        decoder,
        converter,
        resampler,
        faderFilter,
        volumeFilter,
        this->_output, 
        NULL
    ) != (gboolean)TRUE) {
        g_printerr ("Elements could not be linked.\n");
        this->_unrefPipeline();
        return;
    }
    
    //define modifiable properties
    this->_volumeControl = gst_interpolation_control_source_new();
    g_object_set(this->_volumeControl, "mode", GST_INTERPOLATION_MODE_LINEAR, NULL);
    gst_object_add_control_binding(
        GST_OBJECT_CAST(volumeFilter), 
        gst_direct_control_binding_new(
            GST_OBJECT_CAST(volumeFilter), 
            "volume", this->_volumeControl
        )
    );
    
    //define bus and callbacks
    auto bus = gst_element_get_bus(this->_bin);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback)eos_cb, this);
    gst_object_unref(bus);

}

GStreamerClient::~GStreamerClient() {
    this->_unrefPipeline();
}

void GStreamerClient::useSource(QString uri) {
    
    //stop
    this->stop();

    //set new source
    g_object_set(this->_sourceFile, "location", uri.toStdString().c_str(), NULL);

    //play
    this->play();
    
}

void GStreamerClient::setVolume(double volume) {
    //convert and limit
    volume = volume / 100;
    if (volume > 1) volume = 1;

    //set new volume
    gst_timed_value_control_source_set((GstTimedValueControlSource*)this->_volumeControl, 0 * GST_SECOND, volume);
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
        throw std::exception("Could not instanciate GStreamer !");
        return;
    }
}