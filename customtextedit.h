#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>
#include <QWheelEvent>
#include <QPropertyAnimation>

class CustomTextEdit : public QTextEdit {
    Q_OBJECT
    
public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    int lastKey() const { return m_lastKey; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;

signals:
    void resized(); 
    void mouseEntered();
    void mouseLeft();
    void cursorPositionChanged();

private:
    bool m_isResizing = false;
    int m_resizeStartY;
    int m_minHeight = 50;
    int m_maxHeight = 120;
    int m_initialHeight = 50;
    int m_lastKey = 0;
    
};

#endif // CUSTOMTEXTEDIT_H