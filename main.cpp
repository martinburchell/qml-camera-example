#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include "cameraqml.h"

const int CAMERA_WIDTH = 640;
const int CAMERA_HEIGHT = 512;

const int PHOTO_WIDTH = 320;
const int PHOTO_HEIGHT = 160;

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestWindow() :
        m_camera(nullptr)
    {
        m_layout = new QVBoxLayout;

        m_button = new QPushButton("Open camera");
        connect(m_button, &QAbstractButton::clicked,
                this, &TestWindow::openCamera);
        m_layout->addWidget(m_button);

        m_camera_label = new QLabel("Camera goes here");
        m_camera_label->setMaximumWidth(CAMERA_WIDTH);
        m_camera_label->setMaximumHeight(CAMERA_HEIGHT);
        m_layout->addWidget(m_camera_label);

        m_photo = new QLabel();
        m_photo->setMaximumWidth(PHOTO_WIDTH);
        m_photo->setMaximumHeight(PHOTO_HEIGHT);
        m_layout->addWidget(m_photo);

        QWidget *window = new QWidget();
        window->setLayout(m_layout);

        setCentralWidget(window);
    }

    void cameraCancelled()
    {
        if (!m_camera) {
            return;
        }

        hideCamera();
    }

    void imageCaptured(const QImage& image)
    {
        if (!m_camera) {
            return;
        }
        m_photo->setPixmap(QPixmap::fromImage(image));

        hideCamera();
    }

    void rawImageCaptured(const QByteArray& data,
                          const QString& extension_without_dot,
                          const QString& mimetype)
    {
        Q_UNUSED(data)
        Q_UNUSED(extension_without_dot)
        Q_UNUSED(mimetype)
        if (!m_camera) {
            return;
        }

        QImage image;
        bool success = image.loadFromData(data);
        if (success) {
            QImage scaled = image.scaled(PHOTO_WIDTH, PHOTO_HEIGHT,
                                         Qt::KeepAspectRatio);
            m_photo->setPixmap(QPixmap::fromImage(scaled));
        }
        hideCamera();
    }

    void openCamera()
    {
        m_camera = new CameraQml();
        QObject::connect(m_camera, &CameraQml::cancelled,
                         this, &TestWindow::cameraCancelled);
        QObject::connect(m_camera, &CameraQml::rawImageCaptured,
                         this, &TestWindow::rawImageCaptured);
        QObject::connect(m_camera, &CameraQml::imageCaptured,
                         this, &TestWindow::imageCaptured);
        showCamera();
    }

    void showCamera()
    {
        auto old_item = m_layout->replaceWidget(m_camera_label, m_camera);
        delete old_item;
        m_camera_label->setVisible(false);
        m_camera->setVisible(true);
    }

    void hideCamera()
    {
        auto old_item = m_layout->replaceWidget(m_camera, m_camera_label);
        delete old_item;
        m_camera_label->setVisible(true);
        m_camera->setVisible(false);
        m_camera->finish();  // close the camera
        m_camera->deleteLater();
        m_camera = nullptr;
    }

protected:
    QPointer<QLabel> m_camera_label;
    QPointer<CameraQml> m_camera;
    QPointer<QLabel> m_photo;
    QPointer<QPushButton> m_button;
    QPointer<QVBoxLayout> m_layout;
};


int main(int argc, char* argv[])
{
    QApplication app(argc,argv);
    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
