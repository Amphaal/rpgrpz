#include <gst/gst.h>
#include <gst/gstmessage.h>

#include <QString>
#include <QApplication>
#include <QDir>

int tuto(int argc, char** argv) {

    //
    QCoreApplication exec(argc, argv);
    auto td = QDir::toNativeSeparators(QDir::currentPath()).toStdString() + "\\gst-plugins";
    qputenv("GST_PLUGIN_PATH", td.c_str());

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Build the pipeline */
    GError* err = NULL;
    auto pipeline = gst_parse_launch("playbin uri=https://www.youtube.com/watch?v=ZbHd-Br5R6Q", &err);

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Wait until error or EOS */
    auto bus = gst_element_get_bus (pipeline);
    auto msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Free resources */
    if (msg != NULL) {
        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR: {
                GError *err = NULL;
                gchar *dbg_info = NULL;

                gst_message_parse_error (msg, &err, &dbg_info);
                g_printerr ("ERROR from element %s: %s\n",
                    GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
                g_error_free (err);
                g_free (dbg_info);
                break;
            }
        }
        gst_message_unref(msg);
    }

    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    return 0;
}