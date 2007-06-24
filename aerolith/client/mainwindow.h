#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
class MainWindow : public QMainWindow
{
  Q_OBJECT



public:
  MainWindow();
private:
  QLineEdit *solutionLE; // this will be submitted (to answers) when pressing enter
  QLineEdit *chatLE; // as will this (to chat)
  QTextEdit *chatText; // this is the chat box
public slots:
  void submitSolutionLEContents();
  void submitChatLEContents();
};



#endif
