#include "userwidget.h"
#include <QApplication>

UserWidget::UserWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

void UserWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (event->pos().x() >= width() - 7) {
            m_isResizing = true;
            m_resizeStartX = event->globalPosition().toPoint().x();
            m_initialWidth = width();
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        }
    }
    QWidget::mousePressEvent(event);
}

void UserWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isResizing) {
        int newWidth = m_initialWidth + (event->globalPosition().x() - m_resizeStartX);
        
        if (m_minWidth > 0) newWidth = std::max(newWidth, m_minWidth);
        if (m_maxWidth > 0) newWidth = std::min(newWidth, m_maxWidth);
        
        setFixedWidth(newWidth);
    } else {
        if (event->pos().x() >= width() - 7) {
            setCursor(Qt::SizeHorCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void UserWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_isResizing && event->button() == Qt::LeftButton) {
        m_isResizing = false;
        QApplication::restoreOverrideCursor();
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void UserWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    emit sizeChanged(event->size());
}