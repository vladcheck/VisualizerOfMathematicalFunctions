#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMouseEvent>
#include <QObject>
#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QStyleOptionTitleBar>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QTextEdit>
#include <QWidget>
#include <userwidget.h>
#include <customtextedit.h>
#include <QLabel>
#include <QPropertyAnimation>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void resizeEvent(QResizeEvent *event) override;
    void on_pushButton_AddFuncBox_clicked();
    void on_pushButton_SavePlot_clicked();
    void on_pushButton_MinimizeUserWidget_clicked();
    void on_pushButton_MaximizeUserWidget_clicked();
    void on_pushButton_Help_clicked();
    void buildGraph();
    void showSaveSuccessMessage(const QString &filePath);
    void closeEvent(QCloseEvent *event) override;
    void createNewTextEdit();
    void setScrollBarStyle(QScrollBar *scrollBar, const QColor &color);
    void setTextEditFocusStyle(CustomTextEdit *textEdit, const QColor &color);
    QIcon flipIcon(const QIcon &originalIcon, bool horizontal, bool vertical);
    QIcon createSvgIcon(const QString &svgPath,
                        const QColor &newColor,
                        const QColor &oldColor = Qt::black,
                        const QSize &size = QSize(24, 24));
    QVector<QColor> generatePastelColors(int);

private:
    Ui::MainWindow *ui;
    bool isEditDragging = false;
    QPoint dragStartPosition;
    QPoint widgetStartPosition;
    CustomTextEdit *textEditMoveMain;
    int currentIndex;
    int oldIndex;
    int totalHeight = 0;
    int deltaY = 0;
    int wheelEventCount = 0;
    std::vector<bool> visibilityFuncs;
    QVector<CustomTextEdit *> funcBoxes;
    QVector<QColor> colors;
    UserWidget *userWidget;
    const int maxFuncs = 10;
    QGraphicsDropShadowEffect *shadowEffect;
    QLabel *Notification = nullptr;
    QTimer *wheelEndTimer;
    double zoomFactor = 1;
    double defaultH = 0.005;
    double threshold = 1;
    int pointCounter = 0;
    double xRangeMin = 0;
    double xRangeMax = 0;
    double yRangeMin = 0;
    double yRangeMax = 0;
    double scaleFactor = 1;
    int maxHeight = 0;
    QScrollArea *scrollArea;
    int scrollStartValue = 0;
    QWidget *topWidget;
    QWidget *funcWidget;
    QCustomPlot *customPlot;
    QPushButton *pushButton_SavePlot;
    QPushButton *pushButton_AddFuncBox;
    QPushButton *pushButton_MinimizeUserWidget;
    QPushButton *pushButton_MaximizeUserWidget;
    QPushButton *pushButton_Help;
    QTextBrowser *helpTextBrowser;
    QVBoxLayout *verticalLayout_FuncBoxes;
    QVBoxLayout *mainLayout;
    QString htmlContent;
    bool isDragging;
    QPoint lastMousePos;
    QPoint lastMousePosition;
    QPoint mPosition;
    bool m_resizing = false;
    QPoint m_resizeStartPos;
    int m_resizeBorderWidth = 5;
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    QString lastSavePath;
    QIcon deleteIcon;
    QIcon hideIcon;
    QIcon showIcon;
    QIcon burgerIcon;
    QIcon saveIcon;
    QIcon leftArrowSquareIcon;
    QIcon rightArrowSquareIcon;
    QIcon plusFuncIcon;
    QIcon questionMarkIcon;
    QIcon logo64Icon;

    QWidget *dragWidget = nullptr;
    QPoint dragOffset;
    QPropertyAnimation *dropAnimation = nullptr;
    QWidget *dropPlaceholder = nullptr;
    int originalDragIndex = -1;

    void startDrag(QWidget *widget, const QPoint &pos);
    void updateDragPosition(const QPoint &pos);
    void finishDrag(const QPoint &pos);
    int getLayoutIndexAtPos(const QPoint &pos) const;
    void createDropPlaceholder();
    void updateDropPlaceholderPosition(int index);
};

#endif // MAINWINDOW_H
