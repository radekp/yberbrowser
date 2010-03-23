#include <QGLWidget>
#include <QGraphicsItem>

#include "ApplicationWindowHost.h"
#include "Settings.h"
#include "ApplicationWindow.h"
#if defined(Q_WS_MAEMO_5)
#include <QtDBus>
#include <QtMaemo5>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/Xatom.h>
#endif

static const int s_applicationWindowHostWidth = 800;
static const int s_applicationWindowHostHeight = 480;

/*!
  \class ApplicationWindowHost holds an \ApplicationWindow in
  non-DUI environment

  This class is responsible for showing the \ApplicationWindow. This
  is an internal implementation detail of \ApplicationWindow

  This class is the \QMainWindow that holds the \ApplicationWindow.  A
  concept corresponding to \QMainWindow is missing from DUI, and
  that's why we need this.

  \ApplicationWindowHost is created by \ApplicationWindow and "owned"
  by by the windowing system until the window is closed.
*/

ApplicationWindowHost::ApplicationWindowHost()
    : m_mainView(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(s_applicationWindowHostWidth, s_applicationWindowHostHeight);

#if defined(Q_WS_MAEMO_5)
    QDBusConnection::systemBus().connect(QString(), MCE_SIGNAL_PATH, MCE_SIGNAL_IF,
                                         MCE_DEVICE_ORIENTATION_SIG,
                                         this,
                                         SLOT(orientationChanged(QString)));
    grabIncreaseDecreaseKeys(this, true);
#endif

}

void ApplicationWindowHost::setApplicationWindow(ApplicationWindow* window)
{
    m_mainView = window;

    if (Settings::instance()->useGL())  {
        QGLFormat format = QGLFormat::defaultFormat();
        format.setSampleBuffers(false);
        QGLWidget *glWidget = new QGLWidget(format);
        glWidget->setAutoFillBackground(false);
        m_mainView->setViewport(glWidget);
    }

    m_mainView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_mainView->setOptimizationFlags(QGraphicsView::DontSavePainterState);

    m_mainView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainView->setFrameShape(QFrame::NoFrame);
    m_mainView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_mainView->setScene(scene);

    setCentralWidget(m_mainView);
}

ApplicationWindowHost::~ApplicationWindowHost()
{

}

#if defined(Q_WS_MAEMO_5)
bool ApplicationWindowHost::event(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::WindowActivate:
        QDBusConnection::systemBus().call(
            QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                           MCE_REQUEST_IF,
                                           MCE_ACCELEROMETER_ENABLE_REQ));
        break;
    case QEvent::WindowDeactivate:
        QDBusConnection::systemBus().call(
            QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                           MCE_REQUEST_IF,
                                           MCE_ACCELEROMETER_DISABLE_REQ));
        break;
    default:
        break;
    }

    return QWidget::event(ev);
}

void ApplicationWindowHost::orientationChanged(const QString &newOrientation)
{
    if (newOrientation == QLatin1String(MCE_ORIENTATION_PORTRAIT))
        setPortrait();
    else
        setLandscape();
}

void ApplicationWindowHost::setLandscape()
{
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 2)
        setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
#else
        setAttribute(Qt::WA_Maemo5ForceLandscapeOrientation, true);
#endif
}

void ApplicationWindowHost::setPortrait()
{
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 2)
        setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
#else
        setAttribute(Qt::WA_Maemo5ForcePortraitOrientation, true);
#endif
}

void ApplicationWindowHost::grabIncreaseDecreaseKeys(QWidget* window, bool grab)
{
    // Tell maemo-status-volume to grab/ungrab increase/decrease keys
    unsigned long val = (grab==true)?1:0;
    Atom atom;
    atom = XInternAtom( QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", 0);
    XChangeProperty (QX11Info::display(),
                     window->winId(),
                     atom,
                     XA_INTEGER,
                     32,
                     PropModeReplace,
                     (unsigned char *) &val,
                     1);
}

#endif
