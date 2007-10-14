#ifndef _PLAYERINFOWIDGET_H_
#define _PLAYERINFOWIDGET_H_

#define NUM_AVATAR_IDS 73 
// 1-73

#include <QtGui>
#include "avatarLabel.h"
#include "ui_playerInfoForm.h"

class SinglePlayerInfoWidget : public QWidget, public Ui::playerInfoForm
{
 Q_OBJECT
   public:
 SinglePlayerInfoWidget(QWidget *parent, quint8);
 void setReadyIndicator();
 void setUpForGameStart();
 void answered(QString answer);
 void setAvatar(quint8 avatarID);
 
 private:
 
 bool occupied;
 quint8 seatNumber;
 
};

class PlayerInfoWidget : public QWidget
{
Q_OBJECT
 public:
  PlayerInfoWidget();
 void clearAndHide();
  void setupForGameStart();
  void answered(QString username, QString answer);
  void addPlayers(QStringList playerList);
  void addPlayer(QString player, bool gameStarted);
  void removePlayer(QString player, bool gameStarted);
  void leaveTable();
  void setReadyIndicator(QString username);
  void setAvatar(QString username, quint8 avatarID);
  void setMyUsername(QString username);
  void setMaxPlayers(quint8 maxPlayers);
  quint8 getMySeat();
 private:
  SinglePlayerInfoWidget* places[6];
  QHash <QString, int> seats;
  QString myUsername;
  quint8 mySeat;
signals:
  void avatarChange(quint8);
public slots:
  void possibleChangeAvatarLeft();
 void possibleChangeAvatarRight();
};

#endif
