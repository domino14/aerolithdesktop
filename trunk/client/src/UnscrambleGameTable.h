#ifndef _UNSCRAMBLE_GAME_TABLE_H_
#define _UNSCRAMBLE_GAME_TABLE_H_

#include <QtCore>
#include <QtGui>
#include "playerInfoWidget.h"
#include "wordsTableWidget.h"
class UnscrambleGameTable : public QWidget
{
  Q_OBJECT

    public:
  UnscrambleGameTable(QWidget* parent = 0, Qt::WindowFlags f = 0);
  void resetTable(quint16, QString, QString, quint8);
  void leaveTable();
  void addPlayer(QString, bool);
  void removePlayer(QString, bool);
  void addPlayers(QStringList);
  

 protected:
  virtual void closeEvent(QCloseEvent*);
 signals:
  void standUp();
  void giveUp();
  void sendStartRequest();
  void avatarChange(quint8);
  void guessSubmitted(QString);
  void chatTable(QString);
  void exitThisTable();
  private slots:
    void enteredGuess();
  void enteredChat();
  void sendPM(QListWidgetItem* item);

 public:
  wordsTableWidget* wordsWidget;
  QLineEdit* solutionLE;
  QLCDNumber* timerDial;
  QPushButton* exitTable;
  QPushButton* giveup;
  QPushButton* start;
  QPushButton* solutions;
  QLabel* wordListInfo;
  PlayerInfoWidget* playerInfoWidget;
  QPushButton* stand;
  QLineEdit* chatLE;
  QTextEdit* tableChat;
  QListWidget* peopleInTable;

};


#endif
