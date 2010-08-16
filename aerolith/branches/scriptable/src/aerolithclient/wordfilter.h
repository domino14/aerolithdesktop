#ifndef WORDFILTER_H
#define WORDFILTER_H

#include <QObject>
#include "ui_wordFilterForm.h"
#include "databasehandler.h"
#include <QtSql>

class WordFilter : public QWidget
{
    Q_OBJECT
public:
    WordFilter(QWidget* parent, Qt::WindowFlags f);
    void setCurrentLexicon(QString);
private:
    Ui::wordFilterForm filterUi;
    QString currentLexicon;
    QStringList currentWordList;
    QStringList originalWordList;
    bool lexiconChanged;

    void filterByWordLengths(int min, int max);
    void displayCurrentList();

public slots:
    void showWidget();
private slots:
    void on_toolButtonWordLength_clicked();
    void on_toolButtonNumVowels_clicked();
    void on_toolButtonContainsExactly_clicked();
    void on_toolButtonContainsAny_clicked();
    void on_toolButtonAlphagramProb_clicked();
    void on_toolButtonAnagram_clicked();
    void on_pushButtonFilter_clicked();
    void on_pushButtonResetList_clicked();
    void on_pushButtonPrintList_clicked();
};

#endif // WORDFILTER_H
