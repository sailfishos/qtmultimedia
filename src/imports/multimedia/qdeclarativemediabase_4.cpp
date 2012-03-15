/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativemediabase_p_4.h"

#include <QtCore/qcoreevent.h>
#include <QtCore/qurl.h>
#include <QtQml/qqmlinfo.h>

#include <qmediaplayercontrol.h>
#include <qmediaservice.h>
#include <private/qmediaserviceprovider_p.h>
#include <qmetadatareadercontrol.h>

#include "qdeclarativemediametadata_p.h"

QT_BEGIN_NAMESPACE


class QDeclarativeMediaBaseObject : public QMediaObject
{
public:
    QDeclarativeMediaBaseObject(QMediaService *service)
        : QMediaObject(0, service)
    {
    }
};

class QDeclarativeMediaBasePlayerControl : public QMediaPlayerControl
{
public:
    QDeclarativeMediaBasePlayerControl(QObject *parent)
        : QMediaPlayerControl(parent)
    {
    }

    QMediaPlayer::State state() const { return QMediaPlayer::StoppedState; }
    QMediaPlayer::MediaStatus mediaStatus() const { return QMediaPlayer::NoMedia; }

    qint64 duration() const { return 0; }
    qint64 position() const { return 0; }
    void setPosition(qint64) {}
    int volume() const { return 0; }
    void setVolume(int) {}
    bool isMuted() const { return false; }
    void setMuted(bool) {}
    int bufferStatus() const { return 0; }
    bool isAudioAvailable() const { return false; }
    bool isVideoAvailable() const { return false; }
    bool isSeekable() const { return false; }
    QMediaTimeRange availablePlaybackRanges() const { return QMediaTimeRange(); }
    qreal playbackRate() const { return 1; }
    void setPlaybackRate(qreal) {}
    QMediaContent media() const { return QMediaContent(); }
    const QIODevice *mediaStream() const { return 0; }
    void setMedia(const QMediaContent &, QIODevice *) {}

    void play() {}
    void pause() {}
    void stop() {}
};


class QDeclarativeMediaBaseMetaDataControl : public QMetaDataReaderControl
{
public:
    QDeclarativeMediaBaseMetaDataControl(QObject *parent)
        : QMetaDataReaderControl(parent)
    {
    }

    bool isMetaDataAvailable() const { return false; }

    QVariant metaData(const QString &) const { return QVariant(); }
    QStringList availableMetaData() const { return QStringList(); }
};

class QDeclarativeMediaBaseAnimation : public QObject
{
public:
    QDeclarativeMediaBaseAnimation(QDeclarativeMediaBase_4 *media)
        : m_media(media)
    {
    }

    void start() { if (!m_timer.isActive()) m_timer.start(500, this); }
    void stop() { m_timer.stop(); }

protected:
    void timerEvent(QTimerEvent *event)
    {
        if (event->timerId() == m_timer.timerId()) {
            event->accept();

            if (m_media->m_playing && !m_media->m_paused)
                emit m_media->positionChanged();
            if (m_media->m_status == QMediaPlayer::BufferingMedia || QMediaPlayer::StalledMedia)
                emit m_media->bufferProgressChanged();
        } else {
            QObject::timerEvent(event);
        }
    }

private:
    QDeclarativeMediaBase_4 *m_media;
    QBasicTimer m_timer;
};

void QDeclarativeMediaBase_4::_q_statusChanged()
{
    if (m_playerControl->mediaStatus() == QMediaPlayer::EndOfMedia && m_runningCount != 0) {
        m_runningCount -= 1;
        m_playerControl->play();
    }

    const QMediaPlayer::MediaStatus oldStatus = m_status;
    const bool wasPlaying = m_playing;
    const bool wasPaused = m_paused;

    const QMediaPlayer::State state = m_playerControl->state();

    m_status = m_playerControl->mediaStatus();

    if (m_complete)
        m_playing = state != QMediaPlayer::StoppedState;

    if (state == QMediaPlayer::PausedState)
        m_paused = true;
    else if (state == QMediaPlayer::PlayingState)
        m_paused = false;

    if (m_status != oldStatus)
        emit statusChanged();

    switch (state) {
    case QMediaPlayer::StoppedState:
        if (wasPlaying) {
            emit stopped();

            if (!m_playing)
                emit playingChanged();
        }
        break;
    case QMediaPlayer::PausedState:
        if (!wasPlaying) {
            emit started();
            if (m_playing)
                emit playingChanged();
        }
        if ((!wasPaused || !wasPlaying) && m_paused)
            emit paused();
        if (!wasPaused && m_paused)
            emit pausedChanged();

        break;

    case QMediaPlayer::PlayingState:
        if (wasPaused && wasPlaying)
            emit resumed();
        else
            emit started();

        if (wasPaused && !m_paused)
            emit pausedChanged();
        if (!wasPlaying && m_playing)
            emit playingChanged();
        break;
    }

    // Check
    if ((m_playing && !m_paused)
            || m_status == QMediaPlayer::BufferingMedia
            || m_status == QMediaPlayer::StalledMedia) {
        m_animation->start();
    }
    else {
        m_animation->stop();
    }
}

QDeclarativeMediaBase_4::QDeclarativeMediaBase_4()
    : m_paused(false)
    , m_playing(false)
    , m_autoLoad(true)
    , m_loaded(false)
    , m_muted(false)
    , m_complete(false)
    , m_loopCount(1)
    , m_runningCount(0)
    , m_position(0)
    , m_vol(1.0)
    , m_playbackRate(1.0)
    , m_mediaService(0)
    , m_playerControl(0)
    , m_qmlObject(0)
    , m_mediaObject(0)
    , m_mediaProvider(0)
    , m_metaDataControl(0)
    , m_animation(0)
    , m_status(QMediaPlayer::NoMedia)
    , m_error(QMediaPlayer::ServiceMissingError)
{
}

QDeclarativeMediaBase_4::~QDeclarativeMediaBase_4()
{
}

void QDeclarativeMediaBase_4::shutdown()
{
    delete m_mediaObject;
    m_metaData.reset();

    if (m_mediaProvider)
        m_mediaProvider->releaseService(m_mediaService);

    delete m_animation;

}

void QDeclarativeMediaBase_4::setObject(QObject *object, const QByteArray &type)
{
    m_qmlObject = object;

    if ((m_mediaProvider = QMediaServiceProvider::defaultServiceProvider()) != 0) {
        m_mediaService = m_mediaProvider->requestService(type);
        if (m_mediaService != 0) {
            m_playerControl = qobject_cast<QMediaPlayerControl *>(
                    m_mediaService->requestControl(QMediaPlayerControl_iid));
            m_metaDataControl = qobject_cast<QMetaDataReaderControl *>(
                    m_mediaService->requestControl(QMetaDataReaderControl_iid));
            m_mediaObject = new QDeclarativeMediaBaseObject(m_mediaService);
        }
    }

    if (m_playerControl) {
        QObject::connect(m_playerControl, SIGNAL(stateChanged(QMediaPlayer::State)),
                object, SLOT(_q_statusChanged()));
        QObject::connect(m_playerControl, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                object, SLOT(_q_statusChanged()));
        QObject::connect(m_playerControl, SIGNAL(mediaChanged(QMediaContent)),
                object, SIGNAL(sourceChanged()));
        QObject::connect(m_playerControl, SIGNAL(durationChanged(qint64)),
                object, SIGNAL(durationChanged()));
        QObject::connect(m_playerControl, SIGNAL(positionChanged(qint64)),
                object, SIGNAL(positionChanged()));
        QObject::connect(m_playerControl, SIGNAL(volumeChanged(int)),
                object, SIGNAL(volumeChanged()));
        QObject::connect(m_playerControl, SIGNAL(mutedChanged(bool)),
                object, SIGNAL(mutedChanged()));
        QObject::connect(m_playerControl, SIGNAL(bufferStatusChanged(int)),
                object, SIGNAL(bufferProgressChanged()));
        QObject::connect(m_playerControl, SIGNAL(seekableChanged(bool)),
                object, SIGNAL(seekableChanged()));
        QObject::connect(m_playerControl, SIGNAL(playbackRateChanged(qreal)),
                object, SIGNAL(playbackRateChanged()));
        QObject::connect(m_playerControl, SIGNAL(error(int,QString)),
                object, SLOT(_q_error(int,QString)));

        m_animation = new QDeclarativeMediaBaseAnimation(this);
        m_error = QMediaPlayer::NoError;
    } else {
        m_playerControl = new QDeclarativeMediaBasePlayerControl(object);
    }

    if (!m_metaDataControl)
        m_metaDataControl = new QDeclarativeMediaBaseMetaDataControl(object);

    m_metaData.reset(new QDeclarativeMediaMetaData(m_metaDataControl));

    QObject::connect(m_metaDataControl, SIGNAL(metaDataChanged()),
            m_metaData.data(), SIGNAL(metaDataChanged()));
}

void QDeclarativeMediaBase_4::componentComplete()
{
    if (!qFuzzyCompare(m_vol, qreal(1.0)))
        m_playerControl->setVolume(m_vol * 100);
    if (m_muted)
        m_playerControl->setMuted(m_muted);
    if (!qFuzzyCompare(m_playbackRate, qreal(1.0)))
        m_playerControl->setPlaybackRate(m_playbackRate);

    if (!m_source.isEmpty() && (m_autoLoad || m_playing)) // Override autoLoad if playing set
        m_playerControl->setMedia(m_source, 0);

    m_complete = true;

    if (m_playing) {
        if (m_position > 0)
            m_playerControl->setPosition(m_position);

        if (m_source.isEmpty()) {
            m_playing = false;

            emit playingChanged();
        } else if (m_paused) {
            m_playerControl->pause();
        } else {
            m_playerControl->play();
        }
    }
}

// Properties

QUrl QDeclarativeMediaBase_4::source() const
{
    return m_source;
}

void QDeclarativeMediaBase_4::setSource(const QUrl &url)
{
    if (url == m_source)
        return;

    m_source = url;
    m_loaded = false;
    if (m_complete && (m_autoLoad || url.isEmpty())) {
        if (m_error != QMediaPlayer::ServiceMissingError && m_error != QMediaPlayer::NoError) {
            m_error = QMediaPlayer::NoError;
            m_errorString = QString();

            emit errorChanged();
        }

        m_playerControl->setMedia(m_source, 0);
        m_loaded = true;
    }
    else
        emit sourceChanged();
}

bool QDeclarativeMediaBase_4::isAutoLoad() const
{
    return m_autoLoad;
}

void QDeclarativeMediaBase_4::setAutoLoad(bool autoLoad)
{
    if (m_autoLoad == autoLoad)
        return;

    m_autoLoad = autoLoad;
    emit autoLoadChanged();
}

int QDeclarativeMediaBase_4::loopCount() const
{
    return m_loopCount;
}

void QDeclarativeMediaBase_4::setLoopCount(int loopCount)
{
    if (loopCount == 0)
        loopCount = 1;
    else if (loopCount < -1)
        loopCount = -1;

    if (m_loopCount == loopCount) {
        return;
    }
    m_loopCount = loopCount;
    emit loopCountChanged();
}

bool QDeclarativeMediaBase_4::isPlaying() const
{
    return m_playing;
}

void QDeclarativeMediaBase_4::setPlaying(bool playing)
{
    if (playing == m_playing)
        return;

    if (m_complete) {
        if (playing) {
            if (!m_autoLoad && !m_loaded) {
                m_playerControl->setMedia(m_source, 0);
                m_playerControl->setPosition(m_position);
                m_loaded = true;
            }

            m_runningCount = m_loopCount - 1;

            if (!m_paused)
                m_playerControl->play();
            else
                m_playerControl->pause();
        } else {
            m_playerControl->stop();
        }
    } else {
        m_playing = playing;
        emit playingChanged();
    }
}

bool QDeclarativeMediaBase_4::isPaused() const
{
    return m_paused;
}

void QDeclarativeMediaBase_4::setPaused(bool paused)
{
    if (m_paused == paused)
        return;

    if (m_complete && m_playing) {
        if (!m_autoLoad && !m_loaded) {
            m_playerControl->setMedia(m_source, 0);
            m_playerControl->setPosition(m_position);
            m_loaded = true;
        }

        if (!paused)
            m_playerControl->play();
        else
            m_playerControl->pause();
    } else {
        m_paused = paused;
        emit pausedChanged();
    }
}

int QDeclarativeMediaBase_4::duration() const
{
    return !m_complete ? 0 : m_playerControl->duration();
}

int QDeclarativeMediaBase_4::position() const
{
    return !m_complete ? m_position : m_playerControl->position();
}

void QDeclarativeMediaBase_4::setPosition(int position)
{
    if (this->position() == position)
        return;

    m_position = position;
    if (m_complete)
        m_playerControl->setPosition(m_position);
    else
        emit positionChanged();
}

qreal QDeclarativeMediaBase_4::volume() const
{
    return !m_complete ? m_vol : qreal(m_playerControl->volume()) / 100;
}

void QDeclarativeMediaBase_4::setVolume(qreal volume)
{
    if (volume < 0 || volume > 1) {
        qmlInfo(m_qmlObject) << m_qmlObject->tr("volume should be between 0.0 and 1.0");
        return;
    }

    if (m_vol == volume)
        return;

    m_vol = volume;

    if (m_complete)
        m_playerControl->setVolume(qRound(volume * 100));
    else
        emit volumeChanged();
}

bool QDeclarativeMediaBase_4::isMuted() const
{
    return !m_complete ? m_muted : m_playerControl->isMuted();
}

void QDeclarativeMediaBase_4::setMuted(bool muted)
{
    if (m_muted == muted)
        return;

    m_muted = muted;

    if (m_complete)
        m_playerControl->setMuted(muted);
    else
        emit mutedChanged();
}

qreal QDeclarativeMediaBase_4::bufferProgress() const
{
    return !m_complete ? 0 : qreal(m_playerControl->bufferStatus()) / 100;
}

bool QDeclarativeMediaBase_4::isSeekable() const
{
    return !m_complete ? false : m_playerControl->isSeekable();
}

qreal QDeclarativeMediaBase_4::playbackRate() const
{
    return m_playbackRate;
}

void QDeclarativeMediaBase_4::setPlaybackRate(qreal rate)
{
    if (m_playbackRate == rate)
        return;

    m_playbackRate = rate;

    if (m_complete)
        m_playerControl->setPlaybackRate(m_playbackRate);
    else
        emit playbackRateChanged();
}

QString QDeclarativeMediaBase_4::errorString() const
{
    return m_errorString;
}

QDeclarativeMediaMetaData *QDeclarativeMediaBase_4::metaData() const
{
    return m_metaData.data();
}

QT_END_NAMESPACE
