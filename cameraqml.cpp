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
    connect(root, SIGNAL(imageCaptured(const QVariant&)),
            this, SLOT(copyPreviewImage(const QVariant&)));
    connect(root, SIGNAL(previewSaved()), this, SLOT(savePreviewImage()));
    // ... we have to use SIGNAL() and SLOT() since C++ has no idea of the
    // provenance of the signal (and whether or not it exists) -- the macros
    // map signals via strings, so this works, but you'll get an error like
    // "QObject::connect: No such signal PhotoPreview_QMLTYPE_2::imageCaptured(const QString&)"
    // if you get the type wrong.
}


void CameraQml::copyPreviewImage(const QVariant& preview)
{
    m_preview = preview.value<QImage>();
}


void CameraQml::savePreviewImage()
{
    emit imageCaptured(m_preview);
}
