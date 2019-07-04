#include "GStreamerClient.h"

/////////////
/////////////

extern "C" {

    static void gst_rpgrpz_read_tags(const GstTagList * list, const gchar * tag, gpointer user_data) {
        g_print (" %s\n", tag);
    }

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
                emit cli->bufferingPercentChanged(percent);

            }
            break;

            case GST_MESSAGE_TAG: {

                //update current tags list
                GstTagList* newTags = nullptr;
                gst_message_parse_tag(msg, &newTags);
                auto old = cli->_currentPlayTags;
                cli->_currentPlayTags = gst_tag_list_merge(cli->_currentPlayTags, newTags, GST_TAG_MERGE_PREPEND);
                gst_tag_list_unref(old);
                gst_tag_list_unref(newTags);

            }
            break;

            case GST_MESSAGE_STATE_CHANGED: {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(cli->_bin)) {
                    if (new_state == GST_STATE_PLAYING) {
                        // GstTagList *tags;
                        // g_signal_emit_by_name (cli->_bin, "get-audio-tags", 0, &tags);
                        // gchar *str;
                        // gst_tag_list_get_string (tags, GST_TAG_AUDIO_CODEC, &str);
                        // g_print ("  codec: %s\n", str);
                        // auto caca = true;
                    }
                }
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

GStreamerClient::GStreamerClient(QObject* parent) : QObject(parent) {
    
    this->_initGst();

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
    if(this->_currentPlayTags) {
        g_object_unref(this->_currentPlayTags);
        this->_currentPlayTags = nullptr;
    }

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
