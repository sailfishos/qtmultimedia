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

#include "jcamera.h"

#include <QtPlatformSupport/private/qjnihelpers_p.h>
#include <qstringlist.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

static jclass g_qtCameraClass = 0;
static QMap<int, JCamera*> g_objectMap;

static bool sizeLessThan(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

// native method for QtCamera.java
static void notifyPictureExposed(JNIEnv* , jobject, int id)
{
    JCamera *obj = g_objectMap.value(id, 0);
    if (obj)
        Q_EMIT obj->pictureExposed();
}

static void notifyPictureCaptured(JNIEnv *env, jobject, int id, jbyteArray data)
{
    JCamera *obj = g_objectMap.value(id, 0);
    if (obj) {
        QByteArray bytes;
        int arrayLength = env->GetArrayLength(data);
        bytes.resize(arrayLength);
        env->GetByteArrayRegion(data, 0, arrayLength, (jbyte*)bytes.data());
        Q_EMIT obj->pictureCaptured(bytes);
    }
}

JCamera::JCamera(int cameraId, jobject cam)
    : QObject()
    , QJNIObject(cam)
    , m_cameraId(cameraId)
    , m_info(0)
    , m_parameters(0)
{
    if (m_jobject) {
        g_objectMap.insert(cameraId, this);

        m_info = new QJNIObject("android/hardware/Camera$CameraInfo");
        callStaticMethod<void>("android/hardware/Camera",
                               "getCameraInfo",
                               "(ILandroid/hardware/Camera$CameraInfo;)V",
                               cameraId, m_info->object());

        QJNILocalRef<jobject> params = callObjectMethod<jobject>("getParameters",
                                                                 "()Landroid/hardware/Camera$Parameters;");
        m_parameters = new QJNIObject(params.object());
    }
}

JCamera::~JCamera()
{
    if (m_jobject)
        g_objectMap.remove(m_cameraId);
    delete m_parameters;
    delete m_info;
}

JCamera *JCamera::open(int cameraId)
{
    QAttachedJNIEnv env;

    QJNILocalRef<jobject> camera = callStaticObjectMethod<jobject>(g_qtCameraClass,
                                                                   "open",
                                                                   "(I)Lorg/qtproject/qt5/android/multimedia/QtCamera;",
                                                                   cameraId);

    if (camera.isNull())
        return 0;
    else
        return new JCamera(cameraId, camera.object());
}

void JCamera::lock()
{
    callMethod<void>("lock");
}

void JCamera::unlock()
{
    callMethod<void>("unlock");
}

void JCamera::reconnect()
{
    callMethod<void>("reconnect");
}

void JCamera::release()
{
    m_previewSize = QSize();
    delete m_parameters;
    m_parameters = 0;
    callMethod<void>("release");
}

JCamera::CameraFacing JCamera::getFacing()
{
    return CameraFacing(m_info->getField<jint>("facing"));
}

int JCamera::getNativeOrientation()
{
    return m_info->getField<jint>("orientation");
}

QSize JCamera::getPreferredPreviewSizeForVideo()
{
    if (!m_parameters || !m_parameters->isValid())
        return QSize();

    QJNILocalRef<jobject> sizeRef = m_parameters->callObjectMethod<jobject>("getPreferredPreviewSizeForVideo",
                                                                            "()Landroid/hardware/Camera$Size;");

    QJNIObject size(sizeRef.object());
    return QSize(size.getField<jint>("width"), size.getField<jint>("height"));
}

QList<QSize> JCamera::getSupportedPreviewSizes()
{
    QList<QSize> list;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jobject> sizeListRef = m_parameters->callObjectMethod<jobject>("getSupportedPreviewSizes",
                                                                                    "()Ljava/util/List;");
        QJNIObject sizeList(sizeListRef.object());
        int count = sizeList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QJNILocalRef<jobject> sizeRef = sizeList.callObjectMethod<jobject>("get",
                                                                               "(I)Ljava/lang/Object;",
                                                                               i);
            QJNIObject size(sizeRef.object());
            list.append(QSize(size.getField<jint>("width"), size.getField<jint>("height")));
        }

        qSort(list.begin(), list.end(), sizeLessThan);
    }

    return list;
}

void JCamera::setPreviewSize(const QSize &size)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_previewSize = size;

    m_parameters->callMethod<void>("setPreviewSize", "(II)V", size.width(), size.height());
    applyParameters();
}

void JCamera::setPreviewTexture(jobject surfaceTexture)
{
    callMethod<void>("setPreviewTexture", "(Landroid/graphics/SurfaceTexture;)V", surfaceTexture);
}

bool JCamera::isZoomSupported()
{
    if (!m_parameters || !m_parameters->isValid())
        return false;

    return m_parameters->callMethod<jboolean>("isZoomSupported");
}

int JCamera::getMaxZoom()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jint>("getMaxZoom");
}

QList<int> JCamera::getZoomRatios()
{
    QList<int> ratios;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jobject> ratioListRef = m_parameters->callObjectMethod<jobject>("getZoomRatios",
                                                                                     "()Ljava/util/List;");
        QJNIObject ratioList(ratioListRef.object());
        int count = ratioList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QJNILocalRef<jobject> zoomRatioRef = ratioList.callObjectMethod<jobject>("get",
                                                                                     "(I)Ljava/lang/Object;",
                                                                                     i);

            QJNIObject zoomRatio(zoomRatioRef.object());
            ratios.append(zoomRatio.callMethod<jint>("intValue"));
        }
    }

    return ratios;
}

int JCamera::getZoom()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jint>("getZoom");
}

void JCamera::setZoom(int value)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setZoom", "(I)V", value);
    applyParameters();
}

int JCamera::getExposureCompensation()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jint>("getExposureCompensation");
}

void JCamera::setExposureCompensation(int value)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setExposureCompensation", "(I)V", value);
    applyParameters();
}

float JCamera::getExposureCompensationStep()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jfloat>("getExposureCompensationStep");
}

int JCamera::getMinExposureCompensation()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jint>("getMinExposureCompensation");
}

int JCamera::getMaxExposureCompensation()
{
    if (!m_parameters || !m_parameters->isValid())
        return 0;

    return m_parameters->callMethod<jint>("getMaxExposureCompensation");
}

QStringList JCamera::getSupportedSceneModes()
{
    QStringList sceneModes;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jobject> sceneModesRef = m_parameters->callObjectMethod<jobject>("getSupportedSceneModes",
                                                                                      "()Ljava/util/List;");

        if (!sceneModesRef.isNull()) {
            QJNIObject sceneModeList(sceneModesRef.object());
            int count = sceneModeList.callMethod<jint>("size");
            for (int i = 0; i < count; ++i) {
                QJNILocalRef<jobject> sceneModeRef = sceneModeList.callObjectMethod<jobject>("get",
                                                                                             "(I)Ljava/lang/Object;",
                                                                                             i);

                QJNIObject sceneMode(sceneModeRef.object());
                sceneModes.append(qt_convertJString(sceneMode.callObjectMethod<jstring>("toString").object()));
            }
        }
    }

    return sceneModes;
}

QString JCamera::getSceneMode()
{
    QString value;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jstring> sceneMode = m_parameters->callObjectMethod<jstring>("getSceneMode",
                                                                                  "()Ljava/lang/String;");
        if (!sceneMode.isNull())
            value = qt_convertJString(sceneMode.object());
    }

    return value;
}

void JCamera::setSceneMode(const QString &value)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setSceneMode",
                                   "(Ljava/lang/String;)V",
                                   qt_toJString(value).object());
    applyParameters();
}

QStringList JCamera::getSupportedWhiteBalance()
{
    QStringList whiteBalancePresets;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jobject> presetsRef = m_parameters->callObjectMethod<jobject>("getSupportedWhiteBalance",
                                                                                   "()Ljava/util/List;");

        if (!presetsRef.isNull()) {
            QJNIObject presets(presetsRef.object());
            int count = presets.callMethod<jint>("size");
            for (int i = 0; i < count; ++i) {
                QJNILocalRef<jobject> wbRef = presets.callObjectMethod<jobject>("get",
                                                                                "(I)Ljava/lang/Object;",
                                                                                i);

                QJNIObject wb(wbRef.object());
                whiteBalancePresets.append(qt_convertJString(wb.callObjectMethod<jstring>("toString").object()));
            }
        }
    }

    return whiteBalancePresets;
}

QString JCamera::getWhiteBalance()
{
    QString value;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jstring> wb = m_parameters->callObjectMethod<jstring>("getWhiteBalance",
                                                                           "()Ljava/lang/String;");
        if (!wb.isNull())
            value = qt_convertJString(wb.object());
    }

    return value;
}

void JCamera::setWhiteBalance(const QString &value)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setWhiteBalance",
                                   "(Ljava/lang/String;)V",
                                   qt_toJString(value).object());
    applyParameters();
}

void JCamera::setRotation(int rotation)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setRotation", "(I)V", rotation);
    applyParameters();
}

QList<QSize> JCamera::getSupportedPictureSizes()
{
    QList<QSize> list;

    if (m_parameters && m_parameters->isValid()) {
        QJNILocalRef<jobject> sizeListRef = m_parameters->callObjectMethod<jobject>("getSupportedPictureSizes",
                                                                                    "()Ljava/util/List;");
        QJNIObject sizeList(sizeListRef.object());
        int count = sizeList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QJNILocalRef<jobject> sizeRef = sizeList.callObjectMethod<jobject>("get",
                                                                               "(I)Ljava/lang/Object;",
                                                                               i);
            QJNIObject size(sizeRef.object());
            list.append(QSize(size.getField<jint>("width"), size.getField<jint>("height")));
        }

        qSort(list.begin(), list.end(), sizeLessThan);
    }

    return list;
}

void JCamera::setPictureSize(const QSize &size)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setPictureSize", "(II)V", size.width(), size.height());
    applyParameters();
}

void JCamera::setJpegQuality(int quality)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setJpegQuality", "(I)V", quality);
    applyParameters();
}

void JCamera::takePicture()
{
    callMethod<void>("takePicture");
}

void JCamera::startPreview()
{
    callMethod<void>("startPreview");
}

void JCamera::stopPreview()
{
    callMethod<void>("stopPreview");
}

void JCamera::applyParameters()
{
    callMethod<void>("setParameters",
                     "(Landroid/hardware/Camera$Parameters;)V",
                     m_parameters->object());
}

static JNINativeMethod methods[] = {
    {"notifyPictureExposed", "(I)V", (void *)notifyPictureExposed},
    {"notifyPictureCaptured", "(I[B)V", (void *)notifyPictureCaptured}
};

bool JCamera::initJNI(JNIEnv *env)
{
    jclass clazz = env->FindClass("org/qtproject/qt5/android/multimedia/QtCamera");
    if (env->ExceptionCheck())
        env->ExceptionClear();

    if (clazz) {
        g_qtCameraClass = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (env->RegisterNatives(g_qtCameraClass,
                                 methods,
                                 sizeof(methods) / sizeof(methods[0])) < 0) {
            return false;
        }
    }

    return true;
}

QT_END_NAMESPACE