#include "mainwindow.h"

QIcon MainWindow::createSvgIcon(const QString &svgPath,
                                const QColor &newColor,
                                const QColor &oldColor,
                                const QSize &size)
{
    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly))
        return QIcon();

    QString svgString = QString::fromUtf8(file.readAll());
    svgString.replace(oldColor.name(), newColor.name());

    QSvgRenderer renderer(svgString.toUtf8());
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);

    return QIcon(pixmap);
}

QIcon MainWindow::flipIcon(const QIcon &originalIcon, bool horizontal = true, bool vertical = false)
{
    QPixmap pixmap = originalIcon.pixmap(originalIcon.availableSizes().first());

    QTransform transform;
    if (horizontal)
        transform.scale(-1, 1);
    if (vertical)
        transform.scale(1, -1);

    QPixmap flipped = pixmap.transformed(transform, Qt::SmoothTransformation);

    return QIcon(flipped);
}

void MainWindow::setScrollBarStyle(QScrollBar *scrollBar, const QColor &color)
{
    QString styleSheet = QString(
                             "QScrollBar:vertical {"
                             "    background-color: rgba(%1, %2, %3, 0.2);"
                             "    width: 6px;"
                             "    border-radius: 4px;"
                             "}"
                             ""
                             "QScrollBar:hover:vertical {"
                             "    background-color: rgba(%1, %2, %3, 0.3);"
                             "}"
                             ""
                             "QScrollBar::handle:vertical {"
                             "    background-color: rgba(%1, %2, %3, 0.45);"
                             "    min-height: 20px;"
                             "    border-radius: 3px;"
                             "}"
                             ""
                             "QScrollBar::handle:vertical:hover {"
                             "    background-color: rgba(%1, %2, %3, 0.6);"
                             "}"
                             ""
                             "QScrollBar::sub-line:vertical,"
                             "QScrollBar::add-line:vertical {"
                             "    height: 0;"
                             "    width: 0;"
                             "}"
                             ""
                             "QScrollBar::up-arrow:vertical,"
                             "QScrollBar::down-arrow:vertical {"
                             "    background: none;"
                             "}"
                             ""
                             "QScrollBar::add-page:vertical,"
                             "QScrollBar::sub-page:vertical {"
                             "    background: none;"
                             "}"
                             ""
                             "QScrollBar::up-arrow:vertical:hover,"
                             "QScrollBar::down-arrow:vertical:hover {"
                             "    background-color: rgba(%1, %2, %3, 0.7);"
                             "}")
                             .arg(color.red())
                             .arg(color.green())
                             .arg(color.blue());

    scrollBar->setStyleSheet(styleSheet);
}

void MainWindow::setTextEditFocusStyle(CustomTextEdit *textEdit, const QColor &color)
{
    const QString styleSheet = QString(
                                   "QTextEdit:focus {"
                                   "   border-bottom: 2px solid rgba(%1, %2, %3, 0.7);"
                                   "}")
                                   .arg(color.red())
                                   .arg(color.green())
                                   .arg(color.blue());
    textEdit->setStyleSheet(styleSheet);
}

QVector<QColor> MainWindow::generatePastelColors(int count)
{
    QVector<QColor> pastelColors;
    QRandomGenerator *random = QRandomGenerator::global();

    for (int i = 0; i < count; ++i)
    {
        int hue = random->bounded(180, 400);
        int saturation = random->bounded(100, 200);
        int value = random->bounded(150, 240);

        QColor color;
        color.setHsv(hue, saturation, value);
        pastelColors.append(color);
    }

    return pastelColors;
}