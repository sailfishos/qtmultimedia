#include "qgstreamervideosinkcontrol_p.h"

#include <gst/gst.h>

QGStreamerVideoSinkControl::QGStreamerVideoSinkControl(QObject *parent)
    : QGStreamerElementControl(parent)
    , m_videoSink(0)
{
}

QGStreamerVideoSinkControl::~QGStreamerVideoSinkControl()
{
    if (m_videoSink)
        gst_object_unref(GST_OBJECT(m_videoSink));
}

GstElement *QGStreamerVideoSinkControl::videoSink()
{
    return m_videoSink;
}

void QGStreamerVideoSinkControl::setElement(GstElement *element)
{
    if (m_videoSink != element) {
        if (m_videoSink)
            gst_object_unref(GST_OBJECT(m_videoSink));
        m_videoSink = element;
        if (m_videoSink)
            gst_object_ref(GST_OBJECT(element));
        emit sinkChanged();
    }
}
