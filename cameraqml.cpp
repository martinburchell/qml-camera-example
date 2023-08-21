/*
    Copyright (C) 2012, University of Cambridge, Department of Psychiatry.
    Created by Rudolf Cardinal (rnc1001@cam.ac.uk).

    This file is part of CamCOPS.

    CamCOPS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CamCOPS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CamCOPS. If not, see <https://www.gnu.org/licenses/>.
*/


#define DEBUG_CAMERA

#define USE_FILE

#include "cameraqml.h"
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QVBoxLayout>



// ============================================================================
// Constructor/destructor
// ============================================================================

CameraQml::CameraQml(QWidget* parent) : QWidget(parent)
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
#endif

    m_qml_view = new QQuickWidget();
    m_qml_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(m_qml_view->engine(), &QQmlEngine::quit,
            this, &CameraQml::cancelled);
    // Just after calling setSource(), calling view->rootObject() can give a
    // nullptr, because it may be loading in the background. So:
    connect(m_qml_view, &QQuickWidget::statusChanged,
            this, &CameraQml::qmlStatusChanged);
    // ... and must set that signal before calling setSource().
    m_qml_view->setSource(
        QUrl(QString("qrc:///resources/camera_qml/camera.qml"))
    );

    auto top_layout = new QVBoxLayout();
    top_layout->addWidget(m_qml_view);
    setLayout(top_layout);
}


// ============================================================================
// Public interface
// ============================================================================

void CameraQml::finish()
{
}


// ============================================================================
// Internals
// ============================================================================

void CameraQml::qmlStatusChanged(const QQuickWidget::Status status)
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
#endif
    if (status == QQuickWidget::Ready) {
        qmlFinishedLoading();
    } else {
        qWarning() << "QML status is unhappy:" << status;
    }
}


void CameraQml::qmlFinishedLoading()
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
#endif
    Q_ASSERT(m_qml_view);
    QQuickItem* root = m_qml_view->rootObject();
    Q_ASSERT(root);
    // It's possible to connect to non-root objects, but it's much cleaner to
    // route from QML child objects up to the QML root object, and then to C++.
    connect(root, SIGNAL(imageSavedToFile(const QString&, const int)),
            this, SLOT(cameraHasCapturedImage(const QString&, const int)));
    connect(root, SIGNAL(fileNoLongerNeeded(const QString&)),
            this, SLOT(deleteSuperfluousFile(const QString&)));
    // ... we have to use SIGNAL() and SLOT() since C++ has no idea of the
    // provenance of the signal (and whether or not it exists) -- the macros
    // map signals via strings, so this works, but you'll get an error like
    // "QObject::connect: No such signal PhotoPreview_QMLTYPE_2::imageCaptured(const QString&)"
    // if you get the type wrong.
}


void CameraQml::deleteFile(const QString& filename) const
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
#endif
    if (!filename.isEmpty()) {
        bool success = QFile::remove(filename);
        qDebug() << "Deleting temporary camera file " << filename
                 << (success ? "... success" : "... FAILED!");
    }
}


void CameraQml::deleteSuperfluousFile(const QString& filename) const
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
#endif
    deleteFile(filename);
}


void CameraQml::cameraHasCapturedImage(const QString& filename, const int orientation)
{
#ifdef DEBUG_CAMERA
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Camera image has arrived via temporary file" << filename;
#endif

    const QFileInfo fileinfo(filename);
    const QString extension_without_dot = fileinfo.suffix();
    const QMimeDatabase mime_db;
    const QMimeType mime_type = mime_db.mimeTypeForFile(filename);
    // ... default method is to use filename and contents
    // ... it will ALWAYS BE VALID, but may be "application/octet-stream" if
    //     Qt doesn't know what it is:
    //     http://doc.qt.io/qt-5/qmimedatabase.html#mimeTypeForFile
    const QString mimetype_name = mime_type.name();

    QFile file(filename);
    if (mimetype_name != "application/octet-stream" &&
            file.open(QIODevice::ReadOnly)) {

        // We know the MIME type (and can read the file), so we can use the
        // higher-performance method.
        const QByteArray data = file.readAll();
        file.close();
        deleteFile(filename);
#ifdef DEBUG_CAMERA
        qDebug() << "Camera image data loaded";
#endif
        emit rawImageCaptured(data, extension_without_dot, mimetype_name, orientation);

    } else {

#ifdef DEBUG_CAMERA
        qDebug() << "Camera image loaded";
#endif
        QImage img;
        img.load(filename);
        deleteFile(filename);
        emit imageCaptured(img, orientation);

    }

    close();
}
