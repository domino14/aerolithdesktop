#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui>

#include "flashcard.h"

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
    Flashcard* cardTop;
    Flashcard* cardBottom;


    void addPic(QGraphicsPixmapItem*);
private slots:
    void on_pushButtonStartQuiz_clicked();
    void on_pushButtonAddCard_clicked();

    void on_toolButtonAddPicture_clicked();
    void on_toolButtonAddText_clicked();
};

#endif // MAINWINDOW_H
