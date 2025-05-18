#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class UserWidget : public QWidget {
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void sizeChanged(QSize newSize);

private:
    bool m_isResizing = false;
    int m_resizeStartX;
    int m_minWidth = 300;
    int m_maxWidth = 1135;
    int m_initialWidth = 300;
};

#endif  // USERWIDGET_H
