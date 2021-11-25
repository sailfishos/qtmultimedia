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

#include "camerabinexposure.h"
#include "camerabinsession.h"
#include <gst/interfaces/photography.h>

#include <QDebug>

#if !GST_CHECK_VERSION(1,0,0)
typedef GstSceneMode GstPhotographySceneMode;
#endif

QT_BEGIN_NAMESPACE

CameraBinExposure::CameraBinExposure(CameraBinSession *session)
    :QCameraExposureControl(session),
     m_session(session)
{
    if (m_session->photography()) {
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_MANUAL] = QCameraExposure::ExposureManual;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_AUTO] = QCameraExposure::ExposureAuto;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_CLOSEUP] = QCameraExposure::ExposureCloseup;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT] = QCameraExposure::ExposurePortrait;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE] = QCameraExposure::ExposureLandscape;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_SPORT] = QCameraExposure::ExposureSports;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_NIGHT] = QCameraExposure::ExposureNight;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_ACTION] = QCameraExposure::ExposureAction;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_NIGHT_PORTRAIT] = QCameraExposure::ExposureNightPortrait;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_THEATRE] = QCameraExposure::ExposureTheatre;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_BEACH] = QCameraExposure::ExposureBeach;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_SNOW] = QCameraExposure::ExposureSnow;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_SUNSET] = QCameraExposure::ExposureSunset;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_STEADY_PHOTO] = QCameraExposure::ExposureSteadyPhoto;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_FIREWORKS] = QCameraExposure::ExposureFireworks;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_PARTY] = QCameraExposure::ExposureParty;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_CANDLELIGHT] = QCameraExposure::ExposureCandlelight;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_BARCODE] = QCameraExposure::ExposureBarcode;
#if GST_CHECK_VERSION(1, 14, 0)
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_BACKLIGHT] = QCameraExposure::ExposureBacklight;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_FLOWERS] = QCameraExposure::ExposureFlowers;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_AR] = QCameraExposure::ExposureAR;
        m_mappedExposureValues[GST_PHOTOGRAPHY_SCENE_MODE_HDR] = QCameraExposure::ExposureHDR;
#endif
    }
}

CameraBinExposure::~CameraBinExposure()
{
}

bool CameraBinExposure::isParameterSupported(ExposureParameter parameter) const
{
    switch (parameter) {
    case QCameraExposureControl::ExposureCompensation:
    case QCameraExposureControl::ISO:
    case QCameraExposureControl::Aperture:
    case QCameraExposureControl::ShutterSpeed:
#ifdef HAVE_GST_PHOTOGRAPHY
    case QCameraExposureControl::ExposureMode:
#endif
        return true;
    default:
        return false;
    }
}

QVariantList CameraBinExposure::supportedParameterRange(ExposureParameter parameter,
                                                        bool *continuous) const
{
    if (continuous)
        *continuous = false;

    QVariantList res;

    switch (parameter) {
    case QCameraExposureControl::ExposureCompensation:
        if (continuous)
            *continuous = true;
        res << -2.0 << 2.0;
        break;
    case QCameraExposureControl::ISO:
#ifdef HAVE_GST_PHOTOGRAPHY
        if (G_IS_OBJECT(m_session->cameraSource()) &&
                G_IS_OBJECT_CLASS(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource()))) &&
                g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource())), "supported-iso-speeds")) {
            GVariant *iso_modes;
            g_object_get(G_OBJECT(m_session->cameraSource()), "supported-iso-speeds", &iso_modes, NULL);

            if (iso_modes) {
                gsize iso_mode_count;
                const gint32 *modes = (const gint32 *)g_variant_get_fixed_array(iso_modes, &iso_mode_count, sizeof(gint32));

                for (gsize i = 0; i < iso_mode_count; i++) {
                    res << modes[i];
                }
                g_variant_unref(iso_modes);
            }
        } else {
            res << 100 << 200 << 400;
        }
#else
        res << 100 << 200 << 400;
#endif
        break;
    case QCameraExposureControl::Aperture:
        res << 2.8;
        break;
#ifdef HAVE_GST_PHOTOGRAPHY
    case QCameraExposureControl::ExposureMode:
        if (G_IS_OBJECT(m_session->cameraSource()) &&
                G_IS_OBJECT_CLASS(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource()))) &&
                g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(m_session->cameraSource())), "supported-scene-modes")) {
            GVariant *exposure_modes;
            g_object_get(G_OBJECT(m_session->cameraSource()), "supported-scene-modes", &exposure_modes, NULL);

            if (exposure_modes) {
                gsize exposure_mode_count;
                const GstPhotographySceneMode *modes = (const GstPhotographySceneMode *)g_variant_get_fixed_array(exposure_modes, &exposure_mode_count, sizeof(GstPhotographySceneMode));

                for (gsize i = 0; i < exposure_mode_count; i++) {
                    QMap<GstPhotographySceneMode, QCameraExposure::ExposureMode>::const_iterator it = m_mappedExposureValues.find(modes[i]);

                    if (it != m_mappedExposureValues.end()) {
                        res << it.value();
                    }
                }
                g_variant_unref(exposure_modes);
            }
        }
        break;
#endif
    default:
        break;
    }

    return res;
}

QVariant CameraBinExposure::requestedValue(ExposureParameter parameter) const
{
    return m_requestedValues.value(parameter);
}

QVariant CameraBinExposure::actualValue(ExposureParameter parameter) const
{
    switch (parameter) {
    case QCameraExposureControl::ExposureCompensation:
    {
        gfloat ev;
        gst_photography_get_ev_compensation(m_session->photography(), &ev);
        return QVariant(ev);
    }
    case QCameraExposureControl::ISO:
    {
        guint isoSpeed = 0;
        gst_photography_get_iso_speed(m_session->photography(), &isoSpeed);
        return QVariant(isoSpeed);
    }
    case QCameraExposureControl::Aperture:
        return QVariant(2.8);
    case QCameraExposureControl::ShutterSpeed:
    {
        guint32 shutterSpeed = 0;
        gst_photography_get_exposure(m_session->photography(), &shutterSpeed);

        return QVariant(shutterSpeed/1000000.0);
    }
    case QCameraExposureControl::ExposureMode:
    {
        GstPhotographySceneMode sceneMode;
        gst_photography_get_scene_mode(m_session->photography(), &sceneMode);

        switch (sceneMode) {
        case GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT:
            return QVariant::fromValue(QCameraExposure::ExposurePortrait);
        case GST_PHOTOGRAPHY_SCENE_MODE_SPORT:
            return QVariant::fromValue(QCameraExposure::ExposureSports);
        case GST_PHOTOGRAPHY_SCENE_MODE_NIGHT:
            return QVariant::fromValue(QCameraExposure::ExposureNight);
        case GST_PHOTOGRAPHY_SCENE_MODE_MANUAL:
            return QVariant::fromValue(QCameraExposure::ExposureManual);
        case GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE:
            return QVariant::fromValue(QCameraExposure::ExposureLandscape);
#if GST_CHECK_VERSION(1, 2, 0)
        case GST_PHOTOGRAPHY_SCENE_MODE_SNOW:
            return QVariant::fromValue(QCameraExposure::ExposureSnow);
        case GST_PHOTOGRAPHY_SCENE_MODE_BEACH:
            return QVariant::fromValue(QCameraExposure::ExposureBeach);
        case GST_PHOTOGRAPHY_SCENE_MODE_ACTION:
            return QVariant::fromValue(QCameraExposure::ExposureAction);
        case GST_PHOTOGRAPHY_SCENE_MODE_NIGHT_PORTRAIT:
            return QVariant::fromValue(QCameraExposure::ExposureNightPortrait);
        case GST_PHOTOGRAPHY_SCENE_MODE_THEATRE:
            return QVariant::fromValue(QCameraExposure::ExposureTheatre);
        case GST_PHOTOGRAPHY_SCENE_MODE_SUNSET:
            return QVariant::fromValue(QCameraExposure::ExposureSunset);
        case GST_PHOTOGRAPHY_SCENE_MODE_STEADY_PHOTO:
            return QVariant::fromValue(QCameraExposure::ExposureSteadyPhoto);
        case GST_PHOTOGRAPHY_SCENE_MODE_FIREWORKS:
            return QVariant::fromValue(QCameraExposure::ExposureFireworks);
        case GST_PHOTOGRAPHY_SCENE_MODE_PARTY:
            return QVariant::fromValue(QCameraExposure::ExposureParty);
        case GST_PHOTOGRAPHY_SCENE_MODE_CANDLELIGHT:
            return QVariant::fromValue(QCameraExposure::ExposureCandlelight);
        case GST_PHOTOGRAPHY_SCENE_MODE_BARCODE:
            return QVariant::fromValue(QCameraExposure::ExposureBarcode);
#endif
#if GST_CHECK_VERSION(1, 14, 0)
        case GST_PHOTOGRAPHY_SCENE_MODE_BACKLIGHT:
            return QVariant::fromValue(QCameraExposure::ExposureBacklight);
        case GST_PHOTOGRAPHY_SCENE_MODE_FLOWERS:
            return QVariant::fromValue(QCameraExposure::ExposureFlowers);
        case GST_PHOTOGRAPHY_SCENE_MODE_AR:
            return QVariant::fromValue(QCameraExposure::ExposureAR);
        case GST_PHOTOGRAPHY_SCENE_MODE_HDR:
            return QVariant::fromValue(QCameraExposure::ExposureHDR);
#endif
        case GST_PHOTOGRAPHY_SCENE_MODE_CLOSEUP:
            return QVariant::fromValue(QCameraExposure::ExposureCloseup);
        case GST_PHOTOGRAPHY_SCENE_MODE_AUTO:
        default:
            return QVariant::fromValue(QCameraExposure::ExposureAuto);
        }
    }
    case QCameraExposureControl::MeteringMode:
        return QCameraExposure::MeteringMatrix;
    default:
        return QVariant();
    }
}

bool CameraBinExposure::setValue(ExposureParameter parameter, const QVariant& value)
{
    QVariant oldValue = actualValue(parameter);

    switch (parameter) {
    case QCameraExposureControl::ExposureCompensation:
        gst_photography_set_ev_compensation(m_session->photography(), value.toReal());
        break;
    case QCameraExposureControl::ISO:
        gst_photography_set_iso_speed(m_session->photography(), value.toInt());
        break;
    case QCameraExposureControl::Aperture:
        gst_photography_set_aperture(m_session->photography(), guint(value.toReal()*1000000));
        break;
    case QCameraExposureControl::ShutterSpeed:
        gst_photography_set_exposure(m_session->photography(), guint(value.toReal()*1000000));
        break;
    case QCameraExposureControl::ExposureMode:
    {
        QCameraExposure::ExposureMode mode = value.value<QCameraExposure::ExposureMode>();
        GstPhotographySceneMode sceneMode;

        gst_photography_get_scene_mode(m_session->photography(), &sceneMode);

        switch (mode) {
        case QCameraExposure::ExposureManual:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_MANUAL;
            break;
        case QCameraExposure::ExposurePortrait:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT;
            break;
        case QCameraExposure::ExposureSports:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SPORT;
            break;
        case QCameraExposure::ExposureNight:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT;
            break;
        case QCameraExposure::ExposureAuto:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_AUTO;
            break;
        case QCameraExposure::ExposureLandscape:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE;
            break;
#if GST_CHECK_VERSION(1, 2, 0)
        case QCameraExposure::ExposureSnow:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SNOW;
            break;
        case QCameraExposure::ExposureBeach:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_BEACH;
            break;
        case QCameraExposure::ExposureAction:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_ACTION;
            break;
        case QCameraExposure::ExposureNightPortrait:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT_PORTRAIT;
            break;
        case QCameraExposure::ExposureTheatre:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_THEATRE;
            break;
        case QCameraExposure::ExposureSunset:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SUNSET;
            break;
        case QCameraExposure::ExposureSteadyPhoto:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_STEADY_PHOTO;
            break;
        case QCameraExposure::ExposureFireworks:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_FIREWORKS;
            break;
        case QCameraExposure::ExposureParty:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_PARTY;
            break;
        case QCameraExposure::ExposureCandlelight:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_CANDLELIGHT;
            break;
        case QCameraExposure::ExposureBarcode:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_BARCODE;
            break;
#endif
#if GST_CHECK_VERSION(1, 14, 0)
        case QCameraExposure::ExposureFlowers:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_FLOWERS;
            break;
        case QCameraExposure::ExposureBacklight:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_BACKLIGHT;
            break;
        case QCameraExposure::ExposureAR:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_AR;
            break;
        case QCameraExposure::ExposureHDR:
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_HDR;
            break;
#endif
        default:
            //Best option is probably to set auto mode for anything unknown
            sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_AUTO;
            break;
        }

        gst_photography_set_scene_mode(m_session->photography(), sceneMode);
        break;
    }
    default:
        return false;
    }

    if (!qFuzzyCompare(m_requestedValues.value(parameter).toReal(), value.toReal())) {
        m_requestedValues[parameter] = value;
        emit requestedValueChanged(parameter);
    }

    QVariant newValue = actualValue(parameter);
    if (!qFuzzyCompare(oldValue.toReal(), newValue.toReal()))
        emit actualValueChanged(parameter);

    return true;
}

QT_END_NAMESPACE
