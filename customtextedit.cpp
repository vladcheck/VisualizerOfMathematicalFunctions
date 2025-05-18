#include "customtextedit.h"
#include <QApplication>

CustomTextEdit::CustomTextEdit(QWidget *parent) : QTextEdit(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground);
    setFocusPolicy(Qt::StrongFocus);
    QTextDocument *doc = document();
    doc->setDocumentMargin(15);
    setMinimumHeight(m_minHeight);
}

void CustomTextEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->pos().y() >= height() - 7)
        {
            m_isResizing = true;
            m_resizeStartY = event->globalPosition().y();
            m_initialHeight = height();
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
            setFocusPolicy(Qt::NoFocus);
            clearFocus();
            setTextInteractionFlags(textInteractionFlags() & ~Qt::TextSelectableByMouse);
        }
    }
    QTextEdit::mousePressEvent(event);
}

void CustomTextEdit::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isResizing)
    {
        int newHeight = m_initialHeight + (event->globalPosition().y() - m_resizeStartY);

        if (m_minHeight > 0)
            newHeight = std::max(newHeight, m_minHeight);
        if (m_maxHeight > 0)
            newHeight = std::min(newHeight, m_maxHeight);

        setFixedHeight(newHeight);
    }
    else
    {
        if (event->pos().y() >= height() - 7)
        {
            viewport()->setCursor(Qt::SizeVerCursor);
        }
        else
        {
            viewport()->setCursor(Qt::IBeamCursor);
        }
    }
    QTextEdit::mouseMoveEvent(event);
}

void CustomTextEdit::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isResizing && event->button() == Qt::LeftButton)
    {
        m_isResizing = false;
        QApplication::restoreOverrideCursor();
        setFocusPolicy(Qt::StrongFocus);
        setFocus();
        setTextInteractionFlags(textInteractionFlags() | Qt::TextSelectableByMouse);
    }
    QTextEdit::mouseReleaseEvent(event);
}

void CustomTextEdit::resizeEvent(QResizeEvent *event) {
    QTextEdit::resizeEvent(event);
    emit resized();
}
void CustomTextEdit::enterEvent(QEnterEvent *event) {
    QTextEdit::enterEvent(event);
    emit mouseEntered();
}

void CustomTextEdit::leaveEvent(QEvent *event) {
    QTextEdit::leaveEvent(event);
    emit mouseLeft();
}
void CustomTextEdit::keyPressEvent(QKeyEvent *event) {
    m_lastKey = event->key();
    QTextEdit::keyPressEvent(event);
    emit cursorPositionChanged();
}

void CustomTextEdit::insertFromMimeData(const QMimeData *source) {
    insertPlainText(source->text());
}