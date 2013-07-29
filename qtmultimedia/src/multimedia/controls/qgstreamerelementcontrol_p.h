#ifndef QGSTREAMERELEMENTCONTROL_P_H
#define QGSTREAMERELEMENTCONTROL_P_H

#include <QtMultimedia/qmediacontrol.h>

typedef struct _GstElement GstElement;

class Q_MULTIMEDIA_EXPORT QGStreamerElementControl : public QMediaControl
{
    Q_OBJECT
public:
    virtual ~QGStreamerElementControl();

    virtual void setElement(GstElement *element) = 0;

protected:
    QGStreamerElementControl(QObject *parent = 0);
};

#define QGStreamerVideoSinkControl_iid "org.qt-project.qt.gstreamervideosinkcontrol/5.0"

#endif
