
#ifndef QGSTREAMERVIDEOSINKCONTROL_P_H
#define QGSTREAMERVIDEOSINKCONTROL_P_H

#include <private/qgstreamerelementcontrol_p.h>
#include <private/qgstreamervideorendererinterface_p.h>

class QGStreamerVideoSinkControl
        : public QGStreamerElementControl
        , public QGstreamerVideoRendererInterface
{
    Q_OBJECT
    Q_INTERFACES(QGstreamerVideoRendererInterface)
public:
    QGStreamerVideoSinkControl(QObject *parent = 0);
    ~QGStreamerVideoSinkControl();

    GstElement *videoSink();
    void setElement(GstElement *element);

Q_SIGNALS:
    void sinkChanged();
    void readyChanged(bool);

private:
    GstElement *m_videoSink;
};

#endif
