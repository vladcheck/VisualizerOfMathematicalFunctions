#include "customscrollbar.h"

void CustomScrollBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClicked();
    QScrollBar::mouseDoubleClickEvent(event);
}