######################################################################
#
# Qt Multimedia
#
######################################################################

TEMPLATE = subdirs

SUBDIRS += m3u

android {
   SUBDIRS += android
}

blackberry {
    SUBDIRS += blackberry
}

qnx {
    SUBDIRS += qnx
}

win32 {
    SUBDIRS += audiocapture
}

win32 {
    config_directshow: SUBDIRS += directshow
    config_wmf: SUBDIRS += wmf
}

unix:!mac:!android {
    config_gstreamer {
       SUBDIRS += gstreamer
    } else {
        SUBDIRS += audiocapture
    }

    # v4l is turned off because it is not supported in Qt 5
    # !maemo*:SUBDIRS += v4l

    config_pulseaudio {
        SUBDIRS += pulseaudio
    }
}

mac:!simulator {
    SUBDIRS += audiocapture

    config_avfoundation: SUBDIRS += avfoundation

    !ios: SUBDIRS += qt7
}

config_opensles {
    SUBDIRS += opensles
}

config_resourcepolicy {
    SUBDIRS += resourcepolicy
}

