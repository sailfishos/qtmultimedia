TARGET = tst_qsoundeffect

QT += core multimedia-private testlib
CONFIG += no_private_qt_headers_warning

# This is more of a system test
CONFIG += testcase insignificant_test

SOURCES += tst_qsoundeffect.cpp

unix:!mac {
    !contains(QT_CONFIG, pulseaudio) {
        DEFINES += QT_MULTIMEDIA_QMEDIAPLAYER
    }
}

TESTDATA += test.wav
