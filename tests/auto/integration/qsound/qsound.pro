TARGET = tst_qsound

QT += core multimedia-private testlib
CONFIG += no_private_qt_headers_warning

# This is more of a system test
CONFIG += testcase insignificant_test

SOURCES += tst_qsound.cpp

TESTDATA += test.wav
