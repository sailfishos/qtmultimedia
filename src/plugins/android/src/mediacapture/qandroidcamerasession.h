/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QANDROIDCAMERASESSION_H
#define QANDROIDCAMERASESSION_H

#include <qcamera.h>

QT_BEGIN_NAMESPACE

class JCamera;
class QAndroidVideoOutput;

class QAndroidCameraSession : public QObject
{
    Q_OBJECT
public:
    explicit QAndroidCameraSession(QObject *parent = 0);
    ~QAndroidCameraSession();

    void setSelectedCamera(int cameraId) { m_selectedCamera = cameraId; }

    QCamera::State state() const { return m_state; }
    void setState(QCamera::State state);

    QCamera::Status status() const { return m_status; }

    QCamera::CaptureModes captureMode() const { return m_captureMode; }
    void setCaptureMode(QCamera::CaptureModes mode);
    bool isCaptureModeSupported(QCamera::CaptureModes mode) const;

    void setVideoPreview(QAndroidVideoOutput *videoOutput);

Q_SIGNALS:
    void statusChanged(QCamera::Status status);
    void stateChanged(QCamera::State);
    void error(int error, const QString &errorString);
    void captureModeChanged(QCamera::CaptureModes);

private Q_SLOTS:
    void onCameraPaused();
    void onCameraResumed();

private:
    bool open();
    void close(bool deleteCamera = true);

    void startPreview();
    void stopPreview();

    int m_selectedCamera;
    JCamera *m_camera;
    QAndroidVideoOutput *m_videoOutput;

    QCamera::CaptureModes m_captureMode;
    QCamera::State m_state;
    QCamera::Status m_status;
    bool m_previewStarted;
};

QT_END_NAMESPACE

#endif // QANDROIDCAMERASESSION_H
