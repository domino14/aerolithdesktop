#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui>

#include "flashcardtextitem.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsRectItem* cardTop;
    QGraphicsRectItem* cardBottom;
    FlashcardTextItem* textTop;
    FlashcardTextItem* textBottom;
    QGraphicsPixmapItem* gfxTop;
    QGraphicsPixmapItem* gfxBottom;

    void addPic(QGraphicsPixmapItem*);
private slots:
    void on_pushButtonStartQuiz_clicked();
    void on_pushButtonAddCard_clicked();

    void on_toolButtonAddBottomPic_clicked();
    void on_toolButtonAddTopPic_clicked();
};

#endif // MAINWINDOW_H
