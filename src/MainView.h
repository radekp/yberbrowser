#ifndef MainView_h_
#define MainView_h_

#include <QGraphicsView>
#include "MainWindow.h"

class QGraphicsWidget;
class QResizeEvent;

class MainView : public QGraphicsView {
    Q_OBJECT

public:
    MainView(QWidget* parent, Settings);

    void setMainWidget(QGraphicsWidget* widget);

    QGraphicsWidget* mainWidget();

    void resizeEvent(QResizeEvent* event);

private:
    QGraphicsWidget* m_mainWidget;
    Settings m_settings;
};

#endif
