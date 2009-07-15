#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "ui_mainForm.h"
#include <QtGui>
#include <QTimer>
#include <QtSql>

struct Question
{
	QString alphagram;
	QStringList solutions;
	bool correct;
	bool asked;
};

class MainWindow : public QWidget
{
  Q_OBJECT
    
    public:
  MainWindow(QWidget* parent = 0);
 
 private:
  Ui::formMainWidget ui;
	QList <Question> questions;
	quint32 randomSeed;
	quint32 currentQuestionNumber;
	quint32 seenQuestions;
	void displayQuestion();
	void displayAnswers(bool);
	QString curWordList;
	bool currentlyAskingQuestion;
	QTimer* questionTimer;
	int timeLimitSecs, currentTime;
	void askQuestion();
    QTextCursor* textCursor;
	QSqlDatabase wordDb;
	
  private slots:

  void on_pushButtonLoad_clicked();
  void on_pushButtonMark_clicked();
  void on_pushButtonSave_clicked();
  void on_pushButtonNext_clicked();
  void on_pushButtonPrevious_clicked();
  void on_pushButtonWriteMissed_clicked();
	void timerTimedOut();
	

};

#endif
