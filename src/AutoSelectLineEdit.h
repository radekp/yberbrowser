#ifndef AutoSelectLineEdit_h
#define AutoSelectLineEdit_h

#include "yberconfig.h"

#include <QLineEdit>
#include <QTimer>

class AutoSelectLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    AutoSelectLineEdit(QWidget* parent);

Q_SIGNALS:
    void focusChanged(bool);

protected:
    void focusInEvent(QFocusEvent*e);
    void focusOutEvent(QFocusEvent*e);

private:
    QTimer m_selectURLTimer;
};

#endif
