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

#include "qopenslesaudioinput.h"

QT_BEGIN_NAMESPACE

QOpenSLESAudioInput::QOpenSLESAudioInput(const QByteArray &device)
    : m_device(device)
{
}

QOpenSLESAudioInput::~QOpenSLESAudioInput()
{
}

QAudio::Error QOpenSLESAudioInput::error() const
{
    return QAudio::NoError;
}

QAudio::State QOpenSLESAudioInput::state() const
{
    return QAudio::IdleState;
}

void QOpenSLESAudioInput::setFormat(const QAudioFormat &format)
{
    Q_UNUSED(format);
}

QAudioFormat QOpenSLESAudioInput::format() const
{
    return QAudioFormat();
}

void QOpenSLESAudioInput::start(QIODevice *device)
{
    Q_UNUSED(device);
}

QIODevice *QOpenSLESAudioInput::start()
{
    return 0;
}

void QOpenSLESAudioInput::stop()
{

}

int QOpenSLESAudioInput::bytesReady() const
{
    return 0;
}


void QOpenSLESAudioInput::resume()
{

}

void QOpenSLESAudioInput::setVolume(qreal vol)
{
    Q_UNUSED(vol);
}

qreal QOpenSLESAudioInput::volume() const
{
    return 0;
}

void QOpenSLESAudioInput::setBufferSize(int value)
{
    Q_UNUSED(value);
}

int QOpenSLESAudioInput::bufferSize() const
{
    return 0;
}

int QOpenSLESAudioInput::periodSize() const
{
    return 0;
}

void QOpenSLESAudioInput::setNotifyInterval(int ms)
{
    Q_UNUSED(ms);
}

int QOpenSLESAudioInput::notifyInterval() const
{
    return 0;
}

qint64 QOpenSLESAudioInput::processedUSecs() const
{
    return 0;
}

void QOpenSLESAudioInput::suspend()
{

}

qint64 QOpenSLESAudioInput::elapsedUSecs() const
{
    return 0;
}

void QOpenSLESAudioInput::reset()
{

}

QT_END_NAMESPACE
