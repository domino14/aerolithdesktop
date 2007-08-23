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
  void resetTable(quint16, QString);

 protected:
  virtual void closeEvent(QCloseEvent*);

  private slots:


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

};


#endif
