#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customtextedit.h"
#include "customscrollbar.h"

#include <QString>
#include <QMessageBox>
#include <QVector>
#include <QMouseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QGraphicsDropShadowEffect>
#include <QStackedWidget>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>
#include <QBuffer>
#include <QColor>
#include <QRandomGenerator>
#include <QProcess>
#include <QDir>

#include <windows.h>

#include <cmath>
#include <stdexcept>
#include <utility>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>

QString replaceIconPlaceholder(
    const QString &htmlContent,
    const QString &placeholder,
    const QIcon &saveIcon)
{
    QPixmap iconPixmap = saveIcon.pixmap(32, 32);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    if (buffer.open(QIODevice::WriteOnly))
    {
        iconPixmap.save(&buffer, "PNG");
        QString iconBase64 = QString::fromLatin1(byteArray.toBase64().data());
        const QString imgTag = QString("<img src='data:image/png;base64,%1'/>").arg(iconBase64);
        QString result = htmlContent;
        return result.replace(placeholder, imgTag);
    }

    return htmlContent;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), isDragging(false)
{
    ui->setupUi(this);

    this->setMinimumSize(800, 500);
    colors = generatePastelColors(maxFuncs);

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    setWindowTitle("The Visualizer");
    logo64Icon = QIcon(":/resources/icons/icons8-graph-64.png");
    setWindowIcon(logo64Icon);

    for (int i = 0; i < maxFuncs; ++i)
    {
        visibilityFuncs.push_back(true);
    }

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    saveIcon = createSvgIcon(":/resources/icons/save_arrow.svg", QColor(111, 173, 236), Qt::white);
    pushButton_SavePlot = new QPushButton();
    pushButton_SavePlot->setCursor(Qt::PointingHandCursor);
    pushButton_SavePlot->setObjectName("ClassPushButtonTopWidget");
    pushButton_SavePlot->setIcon(saveIcon);
    pushButton_SavePlot->setIconSize(QSize(24, 24));
    pushButton_SavePlot->setToolTip("Save the graph as an image in the Download folder");
    pushButton_SavePlot->setFocusPolicy(Qt::NoFocus);
    connect(pushButton_SavePlot, &QPushButton::clicked, this, &MainWindow::on_pushButton_SavePlot_clicked);

    leftArrowSquareIcon = createSvgIcon(":/resources/icons/left_arrow_square.svg", QColor(111, 173, 236), Qt::white);
    pushButton_MinimizeUserWidget = new QPushButton();
    pushButton_MinimizeUserWidget->setCursor(Qt::PointingHandCursor);
    pushButton_MinimizeUserWidget->setObjectName("ClassPushButtonTopWidget");
    pushButton_MinimizeUserWidget->setIcon(leftArrowSquareIcon);
    pushButton_MinimizeUserWidget->setIconSize(QSize(24, 24));
    pushButton_MinimizeUserWidget->setToolTip("Minimise");
    pushButton_MinimizeUserWidget->setFocusPolicy(Qt::NoFocus);
    connect(pushButton_MinimizeUserWidget, &QPushButton::clicked, this, &MainWindow::on_pushButton_MinimizeUserWidget_clicked);

    rightArrowSquareIcon = flipIcon(createSvgIcon(":/resources/icons/left_arrow_square.svg", QColor(255, 255, 255), Qt::white), 1, 0);
    pushButton_MaximizeUserWidget = new QPushButton();
    pushButton_MaximizeUserWidget->setCursor(Qt::PointingHandCursor);
    pushButton_MaximizeUserWidget->setObjectName("PushButtonMaximize");
    pushButton_MaximizeUserWidget->setIcon(rightArrowSquareIcon);
    pushButton_MaximizeUserWidget->setIconSize(QSize(36, 36));
    pushButton_MaximizeUserWidget->setToolTip("Maximize");
    connect(pushButton_MaximizeUserWidget, &QPushButton::clicked, this, &MainWindow::on_pushButton_MaximizeUserWidget_clicked);
    pushButton_MaximizeUserWidget->lower();

    deleteIcon = createSvgIcon(":/resources/icons/delete.svg", QColor(255, 0, 0), Qt::black);
    hideIcon = createSvgIcon(":/resources/icons/hide.svg", QColor(0, 0, 0), Qt::black);
    showIcon = createSvgIcon(":/resources/icons/show.svg", QColor(0, 0, 0), Qt::black);
    burgerIcon = createSvgIcon(":/resources/icons/burger.svg", QColor(200, 200, 200), Qt::black);

    customPlot = new QCustomPlot(centralWidget);
    customPlot->setEnabled(true);
    customPlot->setMouseTracking(true);
    customPlot->move(0, 0);
    customPlot->xAxis->setTickLabels(true);
    customPlot->yAxis->setTickLabels(true);
    customPlot->xAxis->ticker()->setTickCount(20);
    customPlot->yAxis->ticker()->setTickCount(20);
    customPlot->setBackground(QBrush(QColor(255, 255, 255)));
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iSelectPlottables);
    customPlot->installEventFilter(this);
    customPlot->xAxis->setRange(-20, 20);
    customPlot->yAxis->setRange(-20, 20);
    customPlot->setBufferDevicePixelRatio(1);
    customPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::black, 1));
    customPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::black, 1));

    QCPAxisRect *axisRect = customPlot->axisRect();
    axisRect->setMargins(QMargins(0, 0, 0, 0));
    axisRect->setAutoMargins(QCP::msNone);

    customPlot->xAxis->setTickLabelSide(QCPAxis::lsInside);
    customPlot->xAxis->setTickLabelRotation(0);
    customPlot->xAxis->setTickLength(10, 5);
    customPlot->yAxis->setTickLabelSide(QCPAxis::lsInside);
    customPlot->yAxis->setTickLabelRotation(0);
    customPlot->yAxis->setTickLength(10, 5);

    connect(customPlot, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event)
            {
        if (event->button() == Qt::LeftButton) {
        } });

    connect(customPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent *event)
            {
        if (event->button() == Qt::LeftButton) {
            isDragging = true;
            lastMousePos = event->pos();
            customPlot->setPlottingHints(QCP::phImmediateRefresh);
            customPlot->setAntialiasedElements(QCP::aeAll);
            customPlot->setNotAntialiasedElements(QCP::aeAll);
            customPlot->setNoAntialiasingOnDrag(true);
        } });

    connect(customPlot, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event)
            {
        if (event->button() == Qt::LeftButton) {
            customPlot->setPlottingHints(QCP::phNone);
            customPlot->setNotAntialiasedElements(QCP::aeNone);
            customPlot->setAntialiasedElements(QCP::aeNone);
            customPlot->setNoAntialiasingOnDrag(false);
            isDragging = false;
            buildGraph();
        } });

    userWidget = new UserWidget();
    QVBoxLayout *userLayout = new QVBoxLayout(userWidget);
    userWidget->setObjectName("userWidget");
    userLayout->setContentsMargins(0, 0, 0, 0);
    userLayout->setSpacing(0);
    userWidget->setFixedWidth(300);
    userWidget->setMinimumHeight(height());

    shadowEffect = new QGraphicsDropShadowEffect(userWidget);
    shadowEffect->setOffset(10, 0);
    shadowEffect->setColor(QColor(0, 0, 0, 50));
    shadowEffect->setBlurRadius(100);
    userWidget->setGraphicsEffect(shadowEffect);

    topWidget = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topWidget->setObjectName("topWidget");
    topWidget->setFocusPolicy(Qt::NoFocus);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(pushButton_SavePlot);
    topLayout->addWidget(pushButton_MinimizeUserWidget);

    plusFuncIcon = createSvgIcon(":/resources/icons/plus.svg", QColor(111, 173, 236), Qt::white);
    pushButton_AddFuncBox = new QPushButton("Add Function");
    pushButton_AddFuncBox->setCursor(Qt::PointingHandCursor);
    pushButton_AddFuncBox->setObjectName("pushButton_AddFuncBox");
    pushButton_AddFuncBox->setIcon(plusFuncIcon);
    pushButton_AddFuncBox->setIconSize(QSize(24, 24));
    pushButton_AddFuncBox->setToolTip("Add a new function (Ctrl + E)");
    pushButton_AddFuncBox->setFocusPolicy(Qt::NoFocus);
    connect(pushButton_AddFuncBox, &QPushButton::clicked, this, &MainWindow::on_pushButton_AddFuncBox_clicked);

    questionMarkIcon = createSvgIcon(":/resources/icons/question-mark-circled.svg", QColor(111, 173, 236), Qt::black);
    pushButton_Help = new QPushButton();
    pushButton_Help->setCursor(Qt::PointingHandCursor);
    pushButton_Help->setObjectName("pushButton_Help");
    pushButton_Help->setIcon(questionMarkIcon);
    pushButton_Help->setIconSize(QSize(24, 24));
    pushButton_Help->setToolTip("Help");
    pushButton_Help->setFocusPolicy(Qt::NoFocus);
    connect(pushButton_Help, &QPushButton::clicked, this, &MainWindow::on_pushButton_Help_clicked);

    funcWidget = new QWidget();
    verticalLayout_FuncBoxes = new QVBoxLayout(funcWidget);
    verticalLayout_FuncBoxes->setContentsMargins(0, 0, 0, 0);
    verticalLayout_FuncBoxes->setSpacing(0);
    funcWidget->setFocusPolicy(Qt::NoFocus);
    funcWidget->installEventFilter(this);

    connect(userWidget, &UserWidget::sizeChanged, this, [=](QSize newSize)
            {
        funcWidget->setFixedWidth(newSize.width());
        scrollArea->setFixedWidth(newSize.width()); });

    scrollArea = new QScrollArea();
    scrollArea->setObjectName("scrollAreaFuncWidget");
    scrollArea->setWidget(funcWidget);
    scrollArea->setWidgetResizable(false);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    userLayout->addWidget(topWidget);
    userLayout->addWidget(scrollArea);
    userLayout->addWidget(pushButton_AddFuncBox);
    userLayout->addStretch();

    funcWidget->adjustSize();
    scrollArea->updateGeometry();

    pushButton_MaximizeUserWidget->setParent(centralWidget);
    pushButton_MaximizeUserWidget->setFixedSize(30, 30);
    pushButton_MaximizeUserWidget->move(8, 10);

    helpTextBrowser = new QTextBrowser();
    helpTextBrowser->setObjectName("helpTextBrowser");
    helpTextBrowser->setFocusPolicy(Qt::NoFocus);
    helpTextBrowser->setFrameShape(QFrame::NoFrame);
    helpTextBrowser->setFrameShadow(QFrame::Plain);
    helpTextBrowser->setOpenExternalLinks(true);
    helpTextBrowser->setReadOnly(true);
    helpTextBrowser->setOpenLinks(true);

    QFile fStyle(":/resources/help/help.css");
    QString styleSheet;
    fStyle.open(QIODevice::ReadOnly | QIODevice::Text);
    styleSheet = fStyle.readAll();
    fStyle.close();
    QFile helpFile(":/resources/help/help.html");
    if (helpFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&helpFile);
        htmlContent = stream.readAll();
        helpFile.close();
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--DELETE_ICON-->", deleteIcon);
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--MOVE_ICON-->", burgerIcon);
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--HIDE_ICON-->", hideIcon);
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--SHOW_ICON-->", showIcon);
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--RIGHT_ARROW_ICON-->",
                                             flipIcon(createSvgIcon(":/resources/icons/left_arrow_square.svg", QColor(111, 173, 236), Qt::white), 1, 0));
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--LEFT_ARROW_ICON-->", leftArrowSquareIcon);
        htmlContent = replaceIconPlaceholder(htmlContent, "<!--SAVE_ICON-->", saveIcon);

        helpTextBrowser->document()->setBaseUrl(QUrl("qrc:/resources/help/"));
        QString htmlWithStyle = "<style>" + styleSheet + "</style>" + htmlContent;
        helpTextBrowser->setHtml(htmlWithStyle);
    }
    else
    {
        helpTextBrowser->setHtml("<html><body><h1>Help content not available</h1></body></html>");
    }
    helpTextBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    helpTextBrowser->resize(800, 500);
    helpTextBrowser->setWindowTitle("Help");
    helpTextBrowser->setWindowIcon(logo64Icon);
    helpTextBrowser->hide();

    pushButton_Help->setParent(centralWidget);
    pushButton_Help->setFixedSize(32, 32);
    pushButton_Help->move(width() - pushButton_Help->width() - 8, height() - pushButton_Help->height() - 8);

    mainLayout->addWidget(userWidget);
    userWidget->raise();

    wheelEndTimer = new QTimer(this);
    wheelEndTimer->setSingleShot(true);
    wheelEndTimer->setInterval(200);
    connect(wheelEndTimer, &QTimer::timeout, this, [this]()
            {
        customPlot->setPlottingHints(QCP::phNone);
        customPlot->setAntialiasedElements(QCP::aeAll);
        customPlot->setNotAntialiasedElements(QCP::aeNone);
        customPlot->replot(QCustomPlot::rpImmediateRefresh); });

    QSettings settings;
    lastSavePath = settings.value("lastSavePath",
                                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/The Visualizer")
                       .toString();
    QDir().mkpath(lastSavePath);

    if (settings.contains("windowGeometry"))
    {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
    }

    if (settings.contains("windowState"))
    {
        restoreState(settings.value("windowState").toByteArray());
    }

    if (settings.contains("userWidgetWidth"))
    {
        userWidget->setFixedWidth(settings.value("userWidgetWidth").toInt());
    }

    int size = settings.beginReadArray("textEdits");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        colors[i] = settings.value("color").value<QColor>();
        visibilityFuncs[i] = settings.value("visibility").value<bool>();
        createNewTextEdit();
        if(settings.contains("text")) funcBoxes.last()->setPlainText(settings.value("text").toString());
        if(settings.contains("height")) funcBoxes.last()->setFixedHeight(settings.value("height").toInt());
    }
    settings.endArray();

    if (funcBoxes.empty())
        createNewTextEdit();

    buildGraph();
}

MainWindow::~MainWindow()
{
    delete ui;
    QSettings settings;
    settings.setValue("lastSavePath", lastSavePath);
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("userWidgetWidth", userWidget->width());

    settings.beginWriteArray("textEdits");
    for (int i = 0; i < funcBoxes.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("text", funcBoxes[i]->toPlainText());
        settings.setValue("color", colors[i]);
        settings.setValue("visibility", static_cast<bool>(visibilityFuncs[i]));
        settings.setValue("height", funcBoxes[i]->height());
    }
    settings.endArray();
}

void MainWindow::on_pushButton_Help_clicked()
{
    helpTextBrowser->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (customPlot->graphCount() != 0)
    {
        event->ignore();
        QDialog *dialog = new QDialog(this);
        dialog->setWindowFlags(Qt::FramelessWindowHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setStyleSheet(
            "background-color: rgba(0, 0, 0, 0.85);"
            "color: white;"
            "border: none;"
            "border-radius: 4px;");

        QVBoxLayout *layout = new QVBoxLayout(dialog);

        QLabel *label = new QLabel(tr("Are you sure you want to close the program?"), dialog);
        label->setStyleSheet("font-size: 16px; padding: 10px; background: transparent;");
        label->setAlignment(Qt::AlignCenter);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *yesButton = new QPushButton(tr("Yes"), dialog);
        QPushButton *noButton = new QPushButton(tr("No"), dialog);

        QString buttonStyle =
            "QPushButton {"
            "   background-color: #505050;"
            "   color: white;"
            "   border: 1px solid #606060;"
            "   border-radius: 4px;"
            "   padding: 5px 15px;"
            "   min-width: 80px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #606060;"
            "}";

        yesButton->setStyleSheet(buttonStyle);
        noButton->setStyleSheet(buttonStyle);

        buttonLayout->addStretch();
        buttonLayout->addWidget(yesButton);
        buttonLayout->addWidget(noButton);
        buttonLayout->addStretch();

        layout->addWidget(label);
        layout->addLayout(buttonLayout);
        layout->setContentsMargins(20, 20, 20, 20);

        connect(yesButton, &QPushButton::clicked, [=]()
                {
            event->accept();
            dialog->close(); });

        connect(noButton, &QPushButton::clicked, [=]()
                {
            event->ignore();
            dialog->close(); });

        dialog->adjustSize();
        dialog->move(
            (width() - dialog->width()) / 2,
            (height() - dialog->height() - 50) / 2);

        dialog->exec();
    }
}

void MainWindow::on_pushButton_MinimizeUserWidget_clicked()
{
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(userWidget);
    userWidget->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *positionAnimation = new QPropertyAnimation(userWidget, "pos");
    positionAnimation->setDuration(350);
    positionAnimation->setStartValue(userWidget->pos());
    positionAnimation->setEndValue(QPoint(userWidget->pos().x() - userWidget->width(), userWidget->pos().y()));

    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnimation->setDuration(350);
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);

    positionAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    opacityAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(positionAnimation, &QPropertyAnimation::finished, userWidget, &QWidget::hide);

    positionAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::on_pushButton_MaximizeUserWidget_clicked()
{

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(userWidget);
    userWidget->setGraphicsEffect(opacityEffect);
    userWidget->show();

    opacityEffect->setOpacity(0.0);

    QPropertyAnimation *positionAnimation = new QPropertyAnimation(userWidget, "pos");
    positionAnimation->setDuration(150);
    positionAnimation->setStartValue(QPoint(userWidget->pos().x() - userWidget->width(), userWidget->pos().y()));
    positionAnimation->setEndValue(userWidget->pos());

    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnimation->setDuration(150);
    opacityAnimation->setStartValue(0.0);
    opacityAnimation->setEndValue(1.0);

    positionAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(positionAnimation);
    animationGroup->addAnimation(opacityAnimation);

    connect(animationGroup, &QParallelAnimationGroup::finished, [this]()
            {
        shadowEffect = new QGraphicsDropShadowEffect(userWidget);
        
        shadowEffect->setOffset(0, 0);
        shadowEffect->setColor(QColor(0, 0, 0, 50));
        shadowEffect->setBlurRadius(0);
        userWidget->setGraphicsEffect(shadowEffect);

        QPropertyAnimation *offsetAnimation = new QPropertyAnimation(shadowEffect, "offset");
        offsetAnimation->setDuration(1000);
        offsetAnimation->setStartValue(QPointF(0, 0));
        offsetAnimation->setEndValue(QPointF(10, 0));

        QPropertyAnimation *blurAnimation = new QPropertyAnimation(shadowEffect, "blurRadius");
        blurAnimation->setDuration(150);
        blurAnimation->setStartValue(0);
        blurAnimation->setEndValue(100);

        QParallelAnimationGroup *shadowAnimationGroup = new QParallelAnimationGroup(this);
        shadowAnimationGroup->addAnimation(offsetAnimation);
        shadowAnimationGroup->addAnimation(blurAnimation);

        shadowAnimationGroup->start(); });

    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == customPlot && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);

        const double currentRange = customPlot->xAxis->range().upper - customPlot->xAxis->range().lower;
        if (currentRange < 1e-5 && wheelEvent->angleDelta().y() > 0)
        {
            return true;
        }
        if (currentRange > 1e10 && wheelEvent->angleDelta().y() < 0)
        {
            return true;
        }

        double xMouse = customPlot->xAxis->pixelToCoord(wheelEvent->position().x());
        double yMouse = customPlot->yAxis->pixelToCoord(wheelEvent->position().y());

        customPlot->setPlottingHints(QCP::phImmediateRefresh);
        customPlot->setAntialiasedElements(QCP::aeAll);
        customPlot->setNotAntialiasedElements(QCP::aeAll);
        zoomFactor = 1.05;
        if (wheelEvent->angleDelta().y() > 0)
        {
            zoomFactor = 1.0 / zoomFactor;
        }

        Qt::KeyboardModifiers modifiers = wheelEvent->modifiers();

        if (modifiers & Qt::AltModifier)
        {
            if (wheelEvent->angleDelta().x() > 0)
                zoomFactor = 1.0 / zoomFactor;
            customPlot->yAxis->scaleRange(zoomFactor);
        }
        else if (modifiers & Qt::ControlModifier)
        {
            customPlot->xAxis->scaleRange(zoomFactor);
        }
        else
        {
            customPlot->xAxis->scaleRange(zoomFactor, xMouse);
            customPlot->yAxis->scaleRange(zoomFactor, yMouse);
        }
        defaultH *= zoomFactor;
        threshold *= zoomFactor;
        wheelEndTimer->start();
        buildGraph();
        return true;
    }

    if (obj->property("isMoveButton").toBool())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            isEditDragging = true;
            dragStartPosition = mouseEvent->globalPosition().toPoint();
            scrollStartValue = scrollArea->verticalScrollBar()->value();

            textEditMoveMain = obj->property("associatedTextEdit").value<CustomTextEdit *>();
            textEditMoveMain->raise();

            currentIndex = funcBoxes.indexOf(textEditMoveMain);
            oldIndex = currentIndex;
            maxHeight = 0;
            for(auto i: funcBoxes) maxHeight += i->height();
            maxHeight -= textEditMoveMain->height();
            totalHeight = 0;
            if (textEditMoveMain)
            {
                widgetStartPosition = textEditMoveMain->pos();
            }
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QLayoutItem *item = verticalLayout_FuncBoxes->takeAt(oldIndex);
            verticalLayout_FuncBoxes->insertItem(currentIndex, item);
            if (oldIndex != currentIndex)
            {
                QColor color = colors[oldIndex];
                colors.erase(colors.begin() + oldIndex);
                colors.insert(colors.begin() + currentIndex, color);
                visibilityFuncs.erase(visibilityFuncs.begin() + oldIndex);
                visibilityFuncs.insert(visibilityFuncs.begin() + currentIndex, visibilityFuncs[oldIndex]);
            }
            isEditDragging = false;
            return true;
        }
        else if (isEditDragging && event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint currentPos = mouseEvent->globalPosition().toPoint();
            int scrollDelta = scrollArea->verticalScrollBar()->value() - scrollStartValue;
            deltaY = currentPos.y() - dragStartPosition.y() + scrollDelta;
            const int autoscrollMargin = 50;
            const int autoscrollSpeed = 3;

            QRect visibleRect = scrollArea->viewport()->rect();
            QPoint mousePos = scrollArea->viewport()->mapFromGlobal(currentPos);

            if (mousePos.y() < autoscrollMargin)
            {
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - autoscrollSpeed);
            }

            if (mousePos.y() > visibleRect.height() - autoscrollMargin)
            {
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() + autoscrollSpeed);
            }

            if (currentIndex > 0 && deltaY < totalHeight - 25)
            {
                CustomTextEdit *prevTextEdit = funcBoxes[currentIndex - 1];
                QPropertyAnimation *animation = new QPropertyAnimation(prevTextEdit, "pos");
                animation->setDuration(150);
                animation->setStartValue(prevTextEdit->pos());
                animation->setEndValue(QPoint(prevTextEdit->x(), prevTextEdit->y() + textEditMoveMain->height()));
                animation->setEasingCurve(QEasingCurve::InOutQuad);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                std::swap(funcBoxes[currentIndex], funcBoxes[currentIndex - 1]);
                currentIndex--;
                if (currentIndex > 0)
                {
                    totalHeight -= funcBoxes[currentIndex + 1]->height();
                }
                else
                {
                    totalHeight -= funcBoxes[currentIndex + 1]->height();
                }
            }
            else if (currentIndex < funcBoxes.size() - 1 && deltaY > totalHeight + 25)
            {
                CustomTextEdit *nextTextEdit = funcBoxes[currentIndex + 1];
                QPropertyAnimation *animation = new QPropertyAnimation(nextTextEdit, "pos");
                animation->setDuration(150);
                animation->setStartValue(nextTextEdit->pos());
                animation->setEndValue(QPoint(nextTextEdit->x(), nextTextEdit->y() - textEditMoveMain->height()));
                animation->setEasingCurve(QEasingCurve::InOutQuad);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                std::swap(funcBoxes[currentIndex], funcBoxes[currentIndex + 1]);
                currentIndex++;
                if (currentIndex < funcBoxes.size() - 1)
                {
                    totalHeight += funcBoxes[currentIndex - 1]->height();
                }
                else
                {
                    totalHeight += funcBoxes[currentIndex - 1]->height();
                }
            }
            if (textEditMoveMain)
            {
                textEditMoveMain->move(widgetStartPosition.x(), qMin(qMax(widgetStartPosition.y() + deltaY, 0), maxHeight));
            }
            return true;
        }
    }
    if (obj == funcWidget && event->type() == QEvent::Resize)
    {
        scrollArea->setFixedHeight(std::min(funcWidget->height(), height() - topWidget->height() - pushButton_AddFuncBox->height()));
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    int newHeight = event->size().height();
    int newWidth = event->size().width();
    userWidget->setFixedHeight(newHeight);
    scrollArea->setFixedHeight(std::min(funcWidget->height(), newHeight - topWidget->height() - pushButton_AddFuncBox->height()));
    pushButton_Help->move(width() - pushButton_Help->width() - 8, height() - pushButton_Help->height() - 8);
    customPlot->setFixedHeight(newHeight);
    customPlot->setFixedWidth(newWidth);

    if (Notification)
        Notification->move(
            (width() - Notification->width()) / 2,
            (height() - Notification->height() - 50) / 2);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_E)
    {
        on_pushButton_AddFuncBox_clicked();
        event->accept();
    }
    else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_D)
    {
        customPlot->xAxis->setRange(-20, 20);
        customPlot->yAxis->setRange(-20, 20);
        defaultH = 0.03;
        threshold = 1;
        buildGraph();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::on_pushButton_AddFuncBox_clicked()
{
    if (funcBoxes.length() < maxFuncs)
    {
        createNewTextEdit();
    }
    else
    {
        if (Notification)
        {
            delete Notification;
            Notification = nullptr;
        }

        QString message = tr(
                              "<table cellspacing='0' cellpadding='0' style='height: 36px; border: none;'>"
                              "<tr>"
                              "<td style='width: 36px; vertical-align: middle;'>"
                              "</td>"
                              "<td style='padding-left: 4px; vertical-align: middle; font-size: 16px;'>"
                              "You cannot add more than %1 functions"
                              "</td>"
                              "</tr>"
                              "</table>")
                              .arg(maxFuncs);

        Notification = new QLabel(message, this);
        Notification->setAttribute(Qt::WA_DeleteOnClose);
        Notification->setAlignment(Qt::AlignCenter);
        Notification->setStyleSheet(
            "background-color: rgba(0, 0, 0, 0.55);"
            "color: white;"
            "border: none;"
            "border-radius: 4px; "
            "padding: 10px; ");
        Notification->setOpenExternalLinks(false);
        Notification->setTextFormat(Qt::RichText);
        Notification->adjustSize();
        Notification->move(
            (width() - Notification->width()) / 2,
            (height() - Notification->height() - 50) / 2);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(Notification);
        Notification->setGraphicsEffect(effect);

        QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(effect, "opacity");
        fadeInAnimation->setDuration(500);
        fadeInAnimation->setStartValue(0);
        fadeInAnimation->setEndValue(1);

        QPropertyAnimation *stayAnimation = new QPropertyAnimation(effect, "opacity");
        stayAnimation->setDuration(1000);
        stayAnimation->setStartValue(1);
        stayAnimation->setEndValue(1);

        QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(effect, "opacity");
        fadeOutAnimation->setDuration(2500);
        fadeOutAnimation->setStartValue(1);
        fadeOutAnimation->setEndValue(0);
        connect(fadeOutAnimation, &QPropertyAnimation::finished, Notification, &QWidget::hide);

        QSequentialAnimationGroup *animationGroup = new QSequentialAnimationGroup();
        animationGroup->addAnimation(fadeInAnimation);
        animationGroup->addAnimation(stayAnimation);
        animationGroup->addAnimation(fadeOutAnimation);

        animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
        connect(Notification, &QObject::destroyed, animationGroup, &QAbstractAnimation::stop);
        Notification->show();
    }
}

void MainWindow::showSaveSuccessMessage(const QString &filePath)
{
    if (Notification)
    {
        delete Notification;
        Notification = nullptr;
    }
    QPixmap icon(":/resources/icons/checkmark.svg");
    icon = icon.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    icon.save(&buffer, "PNG");
    QString iconBase64 = QString("data:image/png;base64,%1").arg(QString(byteArray.toBase64()));

    QString folderPath = QFileInfo(filePath).absolutePath();
    QString message = tr(
                          "<table cellspacing='0' cellpadding='0' style='height: 36px; border: none;'>"
                          "<tr>"
                          "<td style='width: 36px; vertical-align: middle;'>"
                          "<img src='%1' width='36' height='36'>"
                          "</td>"
                          "<td style='padding-left: 4px; vertical-align: middle; font-size: 16px;'>"
                          "Image was saved to your <a href='file:///%2' style='color: #6fadec; text-decoration: none;'>Downloads</a> folder"
                          "</td>"
                          "</tr>"
                          "</table>")
                          .arg(iconBase64, folderPath);

    Notification = new QLabel(message, this);
    Notification->setAttribute(Qt::WA_DeleteOnClose);
    Notification->setAlignment(Qt::AlignCenter);
    Notification->setStyleSheet(
        "background-color: rgba(0, 0, 0, 0.55);"
        "border: none;"
        "border-radius: 4px; "
        "padding: 10px; ");
    Notification->setOpenExternalLinks(false);
    Notification->setTextFormat(Qt::RichText);
    Notification->adjustSize();

    Notification->move(
        (width() - Notification->width()) / 2,
        (height() - Notification->height() - 50) / 2);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(Notification);
    Notification->setGraphicsEffect(effect);

    QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(effect, "opacity");
    fadeInAnimation->setDuration(500);
    fadeInAnimation->setStartValue(0);
    fadeInAnimation->setEndValue(1);

    QPropertyAnimation *stayAnimation = new QPropertyAnimation(effect, "opacity");
    stayAnimation->setDuration(1000);
    stayAnimation->setStartValue(1);
    stayAnimation->setEndValue(1);

    QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(effect, "opacity");
    fadeOutAnimation->setDuration(2500);
    fadeOutAnimation->setStartValue(1);
    fadeOutAnimation->setEndValue(0);
    connect(fadeOutAnimation, &QPropertyAnimation::finished, Notification, &QWidget::hide);

    QSequentialAnimationGroup *animationGroup = new QSequentialAnimationGroup();
    animationGroup->addAnimation(fadeInAnimation);
    animationGroup->addAnimation(stayAnimation);
    animationGroup->addAnimation(fadeOutAnimation);

    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
    connect(Notification, &QObject::destroyed, animationGroup, &QAbstractAnimation::stop);
    Notification->show();

    connect(Notification, &QLabel::linkActivated, [filePath](const QString &)
            {
        const QString explorer = "explorer";
        QStringList param;
        if(!QFileInfo(filePath).isDir()){
            param<<QLatin1String("/select,");
        }
        param<<QDir::toNativeSeparators(filePath);
        QProcess::startDetached(explorer,param); });
}

void MainWindow::on_pushButton_SavePlot_clicked()
{

    if (lastSavePath.isEmpty())
    {
        lastSavePath = QDir::homePath();
    }
    QString defaultName = "plot_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Plot"),
        lastSavePath + "/" + defaultName + ".png",
        tr("PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;PDF File (*.pdf)"));

    if (filePath.isEmpty())
    {
        return;
    }

    lastSavePath = QFileInfo(filePath).absolutePath();

    QString extension = QFileInfo(filePath).suffix().toLower();
    bool saveResult = false;

    if (extension == "png")
    {
        saveResult = customPlot->savePng(filePath);
    }
    else if (extension == "jpg" || extension == "jpeg")
    {
        saveResult = customPlot->saveJpg(filePath);
    }
    else if (extension == "pdf")
    {
        saveResult = customPlot->savePdf(filePath);
    }
    else
    {
        filePath += ".png";
        saveResult = customPlot->savePng(filePath);
    }

    if (!saveResult)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to save file to:") + "\n" + filePath);
    }
    else
    {
        showSaveSuccessMessage(filePath);
    }
}
