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

#include "qandroidcamerasession.h"

#include "jcamera.h"
#include "qandroidvideooutput.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

QAndroidCameraSession::QAndroidCameraSession(QObject *parent)
    : QObject(parent)
    , m_selectedCamera(0)
    , m_camera(0)
    , m_videoOutput(0)
    , m_captureMode(QCamera::CaptureViewfinder)
    , m_state(QCamera::UnloadedState)
    , m_status(QCamera::UnloadedStatus)
    , m_previewStarted(false)
{
}

QAndroidCameraSession::~QAndroidCameraSession()
{
    close();
}

void QAndroidCameraSession::setCaptureMode(QCamera::CaptureModes mode)
{
    if (m_captureMode == mode || !isCaptureModeSupported(mode))
        return;

    m_captureMode = mode;

    emit captureModeChanged(mode);
}

bool QAndroidCameraSession::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
    return mode == QCamera::CaptureViewfinder
            || mode == (QCamera::CaptureViewfinder & QCamera::CaptureStillImage)
            || mode == (QCamera::CaptureViewfinder & QCamera::CaptureVideo);
}

void QAndroidCameraSession::setState(QCamera::State state)
{
    if (m_state == state)
        return;

    switch (state) {
    case QCamera::UnloadedState:
        close();
        break;
    case QCamera::LoadedState:
    case QCamera::ActiveState:
        if (!m_camera && !open()) {
            emit error(QCamera::CameraError, QStringLiteral("Failed to open camera"));
            return;
        }
        if (state == QCamera::ActiveState)
            startPreview();
        else if (state == QCamera::LoadedState)
            stopPreview();
        break;
    }

     m_state = state;
     emit stateChanged(m_state);
}

bool QAndroidCameraSession::open()
{
    close();

    m_status = QCamera::LoadingStatus;
    emit statusChanged(m_status);

    m_camera = JCamera::open(m_selectedCamera);

    if (m_camera) {
        m_status = QCamera::LoadedStatus;
        connect(m_camera, SIGNAL(paused()), this, SLOT(onCameraPaused()));
        connect(m_camera, SIGNAL(resumed()), this, SLOT(onCameraResumed()));
        emit opened();
    } else {
        m_status = QCamera::UnavailableStatus;
    }

    emit statusChanged(m_status);

    return m_camera != 0;
}

void QAndroidCameraSession::close(bool deleteCamera)
{
    if (!m_camera)
        return;

    stopPreview();

    m_status = QCamera::UnloadingStatus;
    emit statusChanged(m_status);

    m_camera->release();
    if (deleteCamera) {
        m_camera->destroy();
        delete m_camera;
        m_camera = 0;
    }

    m_status = QCamera::UnloadedStatus;
    emit statusChanged(m_status);
}

void QAndroidCameraSession::setVideoPreview(QAndroidVideoOutput *videoOutput)
{
    if (m_videoOutput)
        m_videoOutput->stop();

    m_videoOutput = videoOutput;
}

void QAndroidCameraSession::startPreview()
{
    if (!m_camera || m_previewStarted)
        return;

    m_status = QCamera::StartingStatus;
    emit statusChanged(m_status);

    if (m_videoOutput) {
        QList<QSize> previewSizes = m_camera->getSupportedPreviewSizes();
        QSize size = previewSizes.last();
        m_videoOutput->setVideoSize(size);
        m_camera->setPreviewSize(size.width(), size.height());
        m_camera->setPreviewTexture(m_videoOutput->surfaceTexture());
    }
    m_camera->startPreview();
    m_previewStarted = true;

    m_status = QCamera::ActiveStatus;
    emit statusChanged(m_status);
}

void QAndroidCameraSession::stopPreview()
{
    if (!m_camera || !m_previewStarted)
        return;

    m_status = QCamera::StoppingStatus;
    emit statusChanged(m_status);

    m_camera->stopPreview();
    if (m_videoOutput)
        m_videoOutput->stop();
    m_previewStarted = false;

    m_status = QCamera::LoadedStatus;
    emit statusChanged(m_status);
}

void QAndroidCameraSession::onCameraPaused()
{
    if (m_camera)
        close(false);
}

void QAndroidCameraSession::onCameraResumed()
{
    m_camera->destroy();
    m_camera->deleteLater();
    m_camera = 0;
    if (open())
        startPreview();
}

QT_END_NAMESPACE
