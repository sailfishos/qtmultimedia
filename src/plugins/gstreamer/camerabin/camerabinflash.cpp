/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "camerabinflash.h"
#include "camerabinsession.h"
#include <gst/interfaces/photography.h>

#include <QDebug>

#if !GST_CHECK_VERSION(1,0,0)
typedef GstFlashMode GstPhotographyFlashMode;
#endif

QT_BEGIN_NAMESPACE

CameraBinFlash::CameraBinFlash(CameraBinSession *session)
    :QCameraFlashControl(session),
     m_session(session)
{
#ifdef HAVE_GST_PHOTOGRAPHY
    m_flashMap[GST_PHOTOGRAPHY_FLASH_MODE_AUTO] = QCameraExposure::FlashAuto;
    m_flashMap[GST_PHOTOGRAPHY_FLASH_MODE_OFF] = QCameraExposure::FlashOff;
    m_flashMap[GST_PHOTOGRAPHY_FLASH_MODE_ON] = QCameraExposure::FlashOn;
    m_flashMap[GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN] = QCameraExposure::FlashFill;
    m_flashMap[GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE] = QCameraExposure::FlashRedEyeReduction;
#endif
}

CameraBinFlash::~CameraBinFlash()
{
}

QCameraExposure::FlashModes CameraBinFlash::flashMode() const
{
    GstPhotographyFlashMode flashMode;
    gst_photography_get_flash_mode(m_session->photography(), &flashMode);

    QCameraExposure::FlashModes modes;
    switch (flashMode) {
    case GST_PHOTOGRAPHY_FLASH_MODE_AUTO: modes |= QCameraExposure::FlashAuto; break;
    case GST_PHOTOGRAPHY_FLASH_MODE_OFF: modes |= QCameraExposure::FlashOff; break;
    case GST_PHOTOGRAPHY_FLASH_MODE_ON: modes |= QCameraExposure::FlashOn; break;
    case GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN: modes |= QCameraExposure::FlashFill; break;
    case GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE: modes |= QCameraExposure::FlashRedEyeReduction; break;
    default:
        modes |= QCameraExposure::FlashAuto;
        break;
    }

    gboolean torchEnabled = false;
    g_object_get(G_OBJECT(m_session->cameraSource()), "video-torch", &torchEnabled, NULL);
    if (torchEnabled)
        modes |= QCameraExposure::FlashTorch;

    return modes;
}

void CameraBinFlash::setFlashMode(QCameraExposure::FlashModes mode)
{
    GstPhotographyFlashMode flashMode;
    gst_photography_get_flash_mode(m_session->photography(), &flashMode);

    if (mode.testFlag(QCameraExposure::FlashAuto)) flashMode = GST_PHOTOGRAPHY_FLASH_MODE_AUTO;
    else if (mode.testFlag(QCameraExposure::FlashOff)) flashMode = GST_PHOTOGRAPHY_FLASH_MODE_OFF;
    else if (mode.testFlag(QCameraExposure::FlashOn)) flashMode = GST_PHOTOGRAPHY_FLASH_MODE_ON;
    else if (mode.testFlag(QCameraExposure::FlashFill)) flashMode = GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN;
    else if (mode.testFlag(QCameraExposure::FlashRedEyeReduction)) flashMode = GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE;

    gst_photography_set_flash_mode(m_session->photography(), flashMode);

    gboolean torchEnabled = false;
    g_object_get(G_OBJECT(m_session->cameraSource()), "video-torch", &torchEnabled, NULL);

    gboolean enableTorch = mode.testFlag(QCameraExposure::FlashTorch);

    if (bool(enableTorch) != bool(torchEnabled)) {
        g_object_set(G_OBJECT(m_session->cameraSource()), "video-torch", enableTorch, NULL);
    }
}

bool CameraBinFlash::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{

    //torch light is allowed only in video capture mode
    if (m_session->captureMode() == QCamera::CaptureVideo) {
        if (mode == QCameraExposure::FlashTorch ||
            mode == QCameraExposure::FlashTorch | QCameraExposure::FlashOff)
            return true;
    }

#ifdef HAVE_GST_PHOTOGRAPHY
    //QList<QCameraExposure::FlashMode> supportedFlash;
    QCameraExposure::FlashModes supportedModes;
    if (G_IS_OBJECT(m_session->cameraSource()) &&
            G_IS_OBJECT_CLASS(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource()))) &&
            g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource())), "supported-flash-modes")) {

        GVariant *flash_modes;
        g_object_get(G_OBJECT(m_session->cameraSource()), "supported-flash-modes", &flash_modes, NULL);

        if (flash_modes) {
            int flash_count = g_variant_n_children(flash_modes);

            for (int i = 0; i < flash_count; i++) {
                GVariant *mode = g_variant_get_child_value(flash_modes, i);
                supportedModes |= m_flashMap[static_cast<GstPhotographyFlashMode>(g_variant_get_int32(mode))];
                g_variant_unref(mode);
            }
        }
        return mode & supportedModes;
    }

#endif

    return  mode == QCameraExposure::FlashOff ||
            mode == QCameraExposure::FlashOn ||
            mode == QCameraExposure::FlashAuto ||
            mode == QCameraExposure::FlashRedEyeReduction ||
            mode == QCameraExposure::FlashFill;
}

bool CameraBinFlash::isFlashReady() const
{
    return true;
}

QT_END_NAMESPACE
