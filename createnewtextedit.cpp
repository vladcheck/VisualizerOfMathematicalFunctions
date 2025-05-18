#include "mainwindow.h"
#include <customtextedit.h>
#include <customscrollbar.h>


void MainWindow::createNewTextEdit(){
    QHBoxLayout *newQHBoxLayout = new QHBoxLayout();
    newQHBoxLayout->setContentsMargins(0, 0, 0, 0);
    CustomTextEdit *newTextEdit = new CustomTextEdit(this);
    newTextEdit->setPlaceholderText("Enter your function here...");

    connect(newTextEdit, &CustomTextEdit::textChanged, this, [this, newTextEdit]() {
        buildGraph();
    });
    
    connect(newTextEdit, &CustomTextEdit::cursorPositionChanged, this, [this, newTextEdit]() {
        int key = newTextEdit->lastKey();

        if (key == Qt::Key_Up) {
            int currentIndex = funcBoxes.indexOf(newTextEdit);
            if(currentIndex > 0)
                funcBoxes[currentIndex-1]->setFocus(); 
        } 
        else if (key == Qt::Key_Down) {
            int currentIndex = funcBoxes.indexOf(newTextEdit);
            if(currentIndex < funcBoxes.size()-1)
                funcBoxes[currentIndex+1]->setFocus(); 
        }
    });


    QPushButton *deletePushButton = new QPushButton(newTextEdit);
    deletePushButton->setCursor(Qt::PointingHandCursor);
    deletePushButton->setIcon(deleteIcon);
    deletePushButton->setObjectName("deletePushButton");
    deletePushButton->setIconSize(QSize(24, 24));
    deletePushButton->setToolTip("Remove function");


    QPushButton *visibilityPushButton = new QPushButton(newTextEdit);
    visibilityPushButton->setCursor(Qt::PointingHandCursor);
    visibilityPushButton->setIcon(visibilityFuncs[funcBoxes.size()] ? showIcon:hideIcon);
    visibilityPushButton->setObjectName("hidePushButton");
    visibilityPushButton->setIconSize(QSize(24, 24));
    visibilityPushButton->setToolTip("Hide function");

    newTextEdit->setLineWrapMode(QTextEdit::FixedPixelWidth);
    
    
    QPushButton *movePushButton = new QPushButton(newTextEdit);
    movePushButton->setCursor(Qt::PointingHandCursor);
    movePushButton->setIcon(burgerIcon);
    movePushButton->setObjectName("movePushButton");
    movePushButton->setIconSize(QSize(24, 24));
    movePushButton->setToolTip("Move widget");
    movePushButton->setProperty("isMoveButton", true);
    movePushButton->setProperty("associatedTextEdit", QVariant::fromValue(newTextEdit));
    movePushButton->setProperty("textEditIndex", funcBoxes.size());

    QLabel *warningLabel = new QLabel(newTextEdit);
    warningLabel->setCursor(Qt::PointingHandCursor);
    warningLabel->setObjectName("warningLabel");
    warningLabel->setStyleSheet(
        "QLabel#warningLabel {"
        "  background-color:rgb(211, 53, 66);"
        "  border-radius: 2px;"
        "  max-height: 10px;"
        "  max-width: 4px;"
        "}"
        "QLabel#warningLabel:hover {"
        "  background-color:rgb(255, 0, 0);"
        "}"
    );
    
    warningLabel->setTextFormat(Qt::RichText);
    warningLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    warningLabel->setOpenExternalLinks(true);
    warningLabel->hide();
    
    warningLabel->setAttribute(Qt::WA_DeleteOnClose);

    connect(newTextEdit, &CustomTextEdit::resized, this, [=]() {
        deletePushButton->move(newTextEdit->width() - 40, (newTextEdit->height() - deletePushButton->height()) / 2);
        movePushButton->move(newTextEdit->width() - 70, (newTextEdit->height() - movePushButton->height()) / 2);
        visibilityPushButton->move(newTextEdit->width() - 100, (newTextEdit->height() - visibilityPushButton->height()) / 2);
        newTextEdit->setLineWrapColumnOrWidth(newTextEdit->width()-82);
        warningLabel->move(5, (newTextEdit->height() - warningLabel->height()) / 2);
        funcWidget->adjustSize();
    });
    
    movePushButton->installEventFilter(this);
    newTextEdit->setProperty("associatedWarningLabel", QVariant::fromValue<QLabel*>(warningLabel));

    deletePushButton->hide();
    movePushButton->hide();
    visibilityPushButton->hide();

    connect(newTextEdit, &CustomTextEdit::mouseEntered, this, [=]() {
        deletePushButton->show();
        movePushButton->show();
        visibilityPushButton->show();
    });

    connect(newTextEdit, &CustomTextEdit::mouseLeft, this, [=]() {
        deletePushButton->hide();
        movePushButton->hide();
        visibilityPushButton->hide();
    });

    connect(deletePushButton, &QPushButton::clicked, this, [=]() {
        visibilityFuncs[funcBoxes.indexOf(newTextEdit)] = true;
        funcBoxes.removeOne(newTextEdit);
        verticalLayout_FuncBoxes->removeItem(newQHBoxLayout);
        newQHBoxLayout->deleteLater();
        newTextEdit->deleteLater();
        deletePushButton->deleteLater();
        buildGraph();
        QTimer::singleShot(0, this, [this](){
            funcWidget->adjustSize();
        });
    });

    connect(visibilityPushButton, &QPushButton::clicked, this, [this, newTextEdit, visibilityPushButton]() {
        int index = funcBoxes.indexOf(newTextEdit);
        visibilityFuncs[index] = !visibilityFuncs[index];
        if(visibilityFuncs[index]){
            visibilityPushButton->setIcon(showIcon);
        }
        else{
            visibilityPushButton->setIcon(hideIcon);
        }
        buildGraph();
    });
    
    CustomScrollBar *scrollBar = new CustomScrollBar(newTextEdit);
    int colorIndex = funcBoxes.size() % colors.size();
    QColor initialColor = colors[colorIndex];

    setScrollBarStyle(scrollBar, initialColor);
    setTextEditFocusStyle(newTextEdit, initialColor);

    QObject::connect(scrollBar, &CustomScrollBar::doubleClicked,
        [this, scrollBar, newTextEdit, colorIndex]() 
    {
        QColorDialog dialog;
        dialog.setStyleSheet(
            "*{"
            "   color: #000;"
            "}"
            "QColorDialog {"
            "   background-color: #f5f5f5;"
            "}"
            
            "QColorDialog QPushButton {"
            "   background-color: rgb(111, 173, 236);"
            "   color: white;"
            "   border-radius: 4px;"
            "   padding: 5px 15px;"
            "   min-width: 80px;"
            "   color: #000;"
            "}"
            
            "QColorDialog QPushButton:hover {"
            "   background-color: rgba(111, 173, 236, 0.9);"
            "}"
            
            "QColorDialog QPushButton:pressed {"
            "   background-color: rgba(111, 173, 236, 0.8);"
            "}"
            
            "QColorDialog QColorLuminancePicker {"
            "   qproperty-luminance: 1.0;"
            "}"
            
            "QColorDialog #qt_colorpicker_hue_picker {"
            "   qproperty-hue: 0.5;"
            "}"
        );
        if (dialog.exec() == QColorDialog::Accepted) {
            QColor color = dialog.selectedColor();
            colors[funcBoxes.indexOf(newTextEdit)] = color;
            setScrollBarStyle(scrollBar, color);
            setTextEditFocusStyle(newTextEdit, color);
            buildGraph();
        }
    });

    newTextEdit->setVerticalScrollBar(scrollBar);
    newTextEdit->setContentsMargins(0, 0, 0, 0);
    newTextEdit->setFixedHeight(50);
    newTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    newTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    newTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    funcBoxes.push_back(newTextEdit);
    newQHBoxLayout->addWidget(newTextEdit);
    verticalLayout_FuncBoxes->addLayout(newQHBoxLayout);
    funcWidget->adjustSize();
}