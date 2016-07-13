/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcamerafastcapturecontrol.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCameraFastCaptureControl
    \since 5.5

    \brief The QCameraFastCaptureControl class is a private class that provides a access to image fast capture implementation.
    \inmodule QtMultimedia

    \ingroup multimedia_control

    The QCameraFastCaptureControl class is a private class that provides a access
    to image fast capture implementation.

    The interface name of QCameraFastCaptureControl is \c com.jollamobile.sailfishos.camerafastcapturecontrol/5.4 as
    defined in QCameraFastCaptureControl_iid.
*/

/*!
    \macro QCameraFastCaptureControl_iid

    \c com.jollamobile.sailfishos.camerafastcapturecontrol/5.4

    Defines the interface name of the QCameraFastCaptureControl class.
*/

/*!
    Constructs a camera image fast capture control with the given \a parent.
*/
QCameraFastCaptureControl::QCameraFastCaptureControl(QObject *parent)
    : QMediaControl(parent)
{
}

/*!
    Destroys a camera image fast capture control.
*/
QCameraFastCaptureControl::~QCameraFastCaptureControl()
{
}

/*!
    \fn QCameraFastCaptureControl::isFastCaptureSupported() const

    Returns whether fast capture is supported by the hardware or not.
*/

/*!
    \fn QCameraFastCaptureControl::isFastCaptureEnabled() const

    Returns whether fast capture is enabled or not.
*/

/*!
    \fn QCameraFastCaptureControl::setFastCaptureEnabled(bool enabled)

    Enables or disables fast capture for images.
*/

#include "moc_qcamerafastcapturecontrol.cpp"

QT_END_NAMESPACE
