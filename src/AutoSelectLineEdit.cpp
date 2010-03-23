#include "AutoSelectLineEdit.h"

// timeout between clicking url bar and marking the url selected
static const int s_urlTapSelectAllTimeout = 200;

AutoSelectLineEdit::AutoSelectLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_selectURLTimer(this)
{
    m_selectURLTimer.setSingleShot(true);
    m_selectURLTimer.setInterval(s_urlTapSelectAllTimeout);
    connect(&m_selectURLTimer, SIGNAL(timeout()), this, SLOT(selectAll()));
}

void AutoSelectLineEdit::focusInEvent(QFocusEvent*e)
{
    QLineEdit::focusInEvent(e);
    m_selectURLTimer.start();
}

void AutoSelectLineEdit::focusOutEvent(QFocusEvent*e)
{
    QLineEdit::focusOutEvent(e);
    m_selectURLTimer.stop();
    emit editCancelled();
    deselect();
}
