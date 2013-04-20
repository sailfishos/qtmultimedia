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
#include <qdebug.h>

QT_BEGIN_NAMESPACE

static jclass g_qtCameraClass = 0;
static QMap<int, JCamera*> g_objectMap;

static bool sizeLessThan(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}


// native methods for QtCamera.java
static void notifyPause(JNIEnv*, jobject, int id)
{
    JCamera *obj = g_objectMap.value(id, 0);
    if (obj)
        Q_EMIT obj->paused();
}

static void notifyResume(JNIEnv*, jobject, int id)
{
    JCamera *obj = g_objectMap.value(id, 0);
    if (obj)
        Q_EMIT obj->resumed();
}

JCamera::JCamera(int cameraId, jobject cam)
    : QObject()
    , QJNIObject(cam)
    , m_cameraId(cameraId)
    , m_parameters(0)
{
    if (m_jobject) {
        g_objectMap.insert(cameraId, this);

        QJNILocalRef<jobject> params = callObjectMethod<jobject>("getParameters",
                                                                 "()Landroid/hardware/Camera$Parameters;");
        m_parameters = new QJNIObject(params.object());
    }
}

JCamera::~JCamera()
{
    delete m_parameters;
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
    delete m_parameters;
    m_parameters = 0;
    callMethod<void>("release");
}

void JCamera::destroy()
{
    if (m_jobject)
        g_objectMap.remove(m_cameraId);
    callMethod<void>("destroy");
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

void JCamera::setPreviewSize(int width, int height)
{
    if (!m_parameters || !m_parameters->isValid())
        return;

    m_parameters->callMethod<void>("setPreviewSize", "(II)V", width, height);
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
    {"notifyPause", "(I)V", (void *)notifyPause},
    {"notifyResume", "(I)V", (void *)notifyResume}
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
