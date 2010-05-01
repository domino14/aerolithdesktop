#include "wordfilter.h"
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>

WordFilter::WordFilter(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    filterUi.setupUi(this);
    lexiconChanged = false;
}

void WordFilter::showWidget()
{
    show();
    if (lexiconChanged)
    {
        currentWordList.clear();
        if (DatabaseHandler::lexiconMap.value(currentLexicon).db_client.isOpen())
        {
            QSqlQuery query(DatabaseHandler::lexiconMap.value(currentLexicon).db_client);
            query.exec("BEGIN TRANSACTION");
            query.exec("SELECT word, front_hooks, back_hooks from words");
            while (query.next())
            {
                currentWordList << query.value(0).toString() + "|" + query.value(1).toString() +
                        "|" + query.value(2).toString();
            }

            query.exec("END TRANSACTION");
            originalWordList = currentWordList;
        }
        displayCurrentList();
    }
}

void WordFilter::displayCurrentList()
{
    filterUi.listWidgetWords->clear();
    for (int i = 0; i < 1000; i++)
    {
        if (i == currentWordList.size()) break;
        filterUi.listWidgetWords->addItem(currentWordList.at(i).section("|", 0, 0));

    }

    if (currentWordList.size() > 1000)
        filterUi.listWidgetWords->addItem(QString("  And %1 more...").arg(currentWordList.size()-1000));

    filterUi.labelWordInfo->setText(QString("List has %1 words").arg(currentWordList.size()));
}

void WordFilter::setDbHandler(DatabaseHandler* dbHandler)
{
    this->dbHandler = dbHandler;
    lexiconChanged = true;
}

void WordFilter::setCurrentLexicon(QString cl)
{
    currentLexicon = cl;
    lexiconChanged = true;
}

void WordFilter::on_toolButtonWordLength_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(0);
}
void WordFilter::on_toolButtonNumVowels_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(1);
}
void WordFilter::on_toolButtonContainsExactly_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(2);
}
void WordFilter::on_toolButtonContainsAny_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(3);
}
void WordFilter::on_toolButtonAlphagramProb_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(4);
}
void WordFilter::on_toolButtonAnagram_clicked()
{
    filterUi.stackedWidget->setCurrentIndex(5);
}

void WordFilter::on_pushButtonFilter_clicked()
{
    switch (filterUi.stackedWidget->currentIndex())
    {
    case 0:
        filterByWordLengths(filterUi.spinBoxMinWL->value(), filterUi.spinBoxMaxWL->value());
        break;




    }
}

void WordFilter::on_pushButtonPrintList_clicked()
{
    // for right now this is ghetto, just save a text list

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           "",
                            tr("Text file (*.txt)"));
    if (fileName == "") return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

     QTextStream out(&file);

     for (int i = 0; i < currentWordList.size(); i++)
     {
         QString line = currentWordList.at(i);
         out << line.section("|", 1, 1).toLower() << "\t"
         << line.section("|", 0, 0) << "\t"
         << line.section("|", 2, 2).toLower() << "\n";
     }



    /*
     QPrinter printer(QPrinter::HighResolution);
     QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           "",
                            tr("PDF file (*.pdf)"));
     if (fileName == "") return;
     printer.setOutputFileName(fileName);
     QPainter painter;
     painter.begin(&printer);

     QFont printFont;
     printFont.setFixedPitch(true);
     painter.setFont(printFont);
//     painter.drawText(1,1, "BLAH");
//painter.drawText(51,51, "BLAH");
     bool donePrinting = false;
     while (donePrinting)
     {
         // Use the painter to draw on the page.






//         if (page != lastPage)
//             printer.newPage();
     }

     painter.end();
*/
}

void WordFilter::on_pushButtonResetList_clicked()
{
    currentWordList = originalWordList;
    displayCurrentList();
}

void WordFilter::filterByWordLengths(int min, int max)
{
    if (min > max) return;
    QStringList copy(currentWordList);
    currentWordList.clear();
    for (int i = 0; i < copy.size(); i++)
    {
        QString word = copy.at(i).section("|", 0, 0);
        if (word.length() >= min && word.length() <= max)
            currentWordList << copy.at(i);
    }

    displayCurrentList();
}
