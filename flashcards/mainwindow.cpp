#include "mainwindow.h"
#include "ui_mainwindow.h"

const int cardWidth = 500;
const int cardHeight = 300;

const int cardSep = 10;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    cardTop = new Flashcard(cardSep, cardSep, cardWidth, cardHeight);
    cardBottom = new Flashcard(cardSep, cardSep *2 + cardHeight, cardWidth, cardHeight);

    ui->graphicsView->setSceneRect(0, 0, cardWidth, cardSep * 2 + cardHeight * 2);

    scene->addItem(cardTop);
    scene->addItem(cardBottom);

}



void MainWindow::on_pushButtonStartQuiz_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::on_pushButtonAddCard_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_toolButtonAddPicture_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this, "Select a picture file");
    QPixmap pix(filename);
    double scaleFactor = 1;
    if (!pix.isNull())
    {
        QSize picSize = pix.size();

        double beforeWidth = picSize.width();

        if (picSize.width() > cardWidth || picSize.height() > cardHeight)
        {
            picSize.scale(cardWidth, cardHeight, Qt::KeepAspectRatio);

            qDebug() << "Scaled:"<< picSize.width() << picSize.height();

            scaleFactor = (double)picSize.width() / beforeWidth;
            qDebug() << "Scalefactor" << scaleFactor;
        }

    }
    else return;





    if (ui->radioButtonBottomCard->isChecked())
        cardBottom->addPicture(pix, scaleFactor);
    else if (ui->radioButtonTopCard->isChecked())
        cardTop->addPicture(pix, scaleFactor);
}

void MainWindow::on_toolButtonAddText_clicked()
{
    QString text = QInputDialog::getText(this, "Type in text", "Type in text");
    if (text.size() != 0)
    {
        if (ui->radioButtonBottomCard->isChecked())
            cardBottom->addText(text);
        else if (ui->radioButtonTopCard->isChecked())
            cardTop->addText(text);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}
