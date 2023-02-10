# =============================================================================
# Parts of Qt
# =============================================================================

QT += multimedia
QT += quick  # for QML, e.g. for camera
QT += quickwidgets  # for QQuickWidget
QT += widgets  # required to #include <QApplication>

# =============================================================================
# Overall configuration
# =============================================================================

CONFIG += mobility
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS


# =============================================================================
# Compiler and linker flags
# =============================================================================

gcc {
    QMAKE_CXXFLAGS += -Werror  # warnings become errors
}

if (gcc | clang):!ios:!android {
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
}

gcc {
    QMAKE_CXXFLAGS += -fvisibility=hidden
}

# =============================================================================
# Build targets
# =============================================================================

TARGET = qml-camera-example
TEMPLATE = app

# -----------------------------------------------------------------------------
# Architecture
# -----------------------------------------------------------------------------

linux : {
    CONFIG += static
}

# =============================================================================
# Source files
# =============================================================================

RESOURCES += \
    resources.qrc

SOURCES += \
    cameraqml.cpp \
    main.cpp \


HEADERS += \
    cameraqml.h \
