#ifndef ApplicationWindow_h
#define ApplicationWindow_h

#include "yberconfig.h"

#if USE(DUI)

#include <DuiApplicationWindow>
class ApplicationWindow :  public DuiApplicationWindow 
{
public:
    void showFullScreen() { show(); }
};


#else

#include <QGraphicsView>
class QWidget;
class QResizeEvent;
class ApplicationWindowHost;
class QMenuBar;


class ApplicationWindow : public QGraphicsView
{
public:
    ApplicationWindow(QWidget* parent=0);
    void setPage(QGraphicsWidget* page);

    void setMenuBar(QMenuBar*);
    void show();
    void showFullScreen();
    bool isFullScreen();

    void setTitle(const QString& title);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    ApplicationWindowHost* m_owner;
    QGraphicsWidget* m_page;
    bool m_isFullScreen;
};

#endif

#endif
