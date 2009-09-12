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
    cardTop = new QGraphicsRectItem(cardSep, cardSep, cardWidth, cardHeight);
    cardBottom = new QGraphicsRectItem(cardSep, cardSep *2 + cardHeight, cardWidth, cardHeight);

    textTop = new FlashcardTextItem(cardTop, scene);
    textBottom = new FlashcardTextItem(cardBottom, scene);

    gfxTop = new QGraphicsPixmapItem(cardTop, scene);
    gfxBottom = new QGraphicsPixmapItem(cardBottom, scene);

//    textTop->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
//    textBottom->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    gfxTop->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    gfxBottom->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

    ui->graphicsView->setSceneRect(0, 0, cardWidth, cardSep * 2 + cardHeight * 2);

    textTop->setPos(cardSep, cardSep);
    textBottom->setPos(cardSep, cardSep + cardSep + cardHeight);
    textTop->setTextWidth(cardWidth);
    textBottom->setTextWidth(cardWidth);
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

void MainWindow::on_toolButtonAddTopPic_clicked()
{
    addPic(gfxTop);
    gfxTop->setPos(cardSep, cardSep);

}

void MainWindow::on_toolButtonAddBottomPic_clicked()
{
    addPic(gfxBottom);

    gfxBottom->setPos(cardSep, cardSep*2 + cardHeight);
}

void MainWindow::addPic(QGraphicsPixmapItem* pixItem)
{
    QString filename = QFileDialog::getOpenFileName(this, "Select a picture file");
    QPixmap pix(filename);
    pixItem->setPixmap(pix);
    QSize picSize = pix.size();

    double beforeWidth = picSize.width();

    if (picSize.width() > cardWidth || picSize.height() > cardHeight)
    {
        picSize.scale(cardWidth, cardHeight, Qt::KeepAspectRatio);

        qDebug() << "Scaled:"<< picSize.width() << picSize.height();

        double scaleFactor = (double)picSize.width() / beforeWidth;
        qDebug() << "Scalefactor" << scaleFactor;
        pixItem->scale(scaleFactor, scaleFactor);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
