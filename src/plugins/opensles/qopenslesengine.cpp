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

#include "qopenslesengine.h"

#define CheckError(message) if (result != SL_RESULT_SUCCESS) { qWarning(message); return; }

Q_GLOBAL_STATIC(QOpenSLESEngine, openslesEngine);

QOpenSLESEngine::QOpenSLESEngine()
    : m_engineObject(0)
    , m_engine(0)
    , m_outputMix(0)
{
    SLresult result;

    result = slCreateEngine(&m_engineObject, 0, 0, 0, 0, 0);
    CheckError("Failed to create engine");

    result = (*m_engineObject)->Realize(m_engineObject, SL_BOOLEAN_FALSE);
    CheckError("Failed to realize engine");

    result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_ENGINE, &m_engine);
    CheckError("Failed to get engine interface");

    result = (*m_engine)->CreateOutputMix(m_engine, &m_outputMix, 0, 0, 0);
    CheckError("Failed to create output mix");

    result = (*m_outputMix)->Realize(m_outputMix, SL_BOOLEAN_FALSE);
    CheckError("Failed to realize output mix");

}

QOpenSLESEngine::~QOpenSLESEngine()
{
    if (m_outputMix)
        (*m_outputMix)->Destroy(m_outputMix);
    if (m_engineObject)
        (*m_engineObject)->Destroy(m_engineObject);
}

QOpenSLESEngine *QOpenSLESEngine::instance()
{
    return openslesEngine();
}

SLDataFormat_PCM QOpenSLESEngine::audioFormatToSLFormatPCM(const QAudioFormat &format)
{
    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = format.channelCount();
    format_pcm.samplesPerSec = format.sampleRate() * 1000;
    format_pcm.bitsPerSample = format.sampleSize();
    format_pcm.containerSize = format.sampleSize();
    format_pcm.channelMask = (format.channelCount() == 1 ?
                                  SL_SPEAKER_FRONT_CENTER :
                                  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);
    format_pcm.endianness = (format.byteOrder() == QAudioFormat::LittleEndian ?
                                 SL_BYTEORDER_LITTLEENDIAN :
                                 SL_BYTEORDER_BIGENDIAN);
    return format_pcm;

}

QList<QByteArray> QOpenSLESEngine::availableDevices(QAudio::Mode mode) const
{
    Q_UNUSED(mode);
    // The Android OpenSL implementation doesn't provide the SLAudioIODeviceCapabilities interface
    // and therefore we can only use the default input and output devices.
    return QList<QByteArray>() << "default";
}
