#ifndef _PLAYERINFOWIDGET_H_
#define _PLAYERINFOWIDGET_H_

#define NUM_AVATAR_IDS 73 
// 1-73

#include <QtGui>

#include "avatarLabel.h"

class SinglePlayerInfoWidget : public QWidget
{
 Q_OBJECT
   public:
 SinglePlayerInfoWidget(quint8);
 void clearAndHide();
 void setReadyIndicator();
 void answered(QString answer);
 void setAvatar(quint8 avatarID);
   private:
 quint8 seatNumber;
 QLabel* playerName;
 avatarLabel* playerAvatar;
 QLabel* playerStatus;
 QListWidget* playerList;
 QToolButton* sitButton;
 QStackedWidget* avatarStack;
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
 private:
  QHBoxLayout* playerInfoLayout[6];
  QListWidget* playerLists[6];
  QLabel* playerNames[6];
  avatarLabel* playerAvatars[6];
  QLabel* playerStatus[6];
  SinglePlayerInfoWidget* places[6];
  QHash <QString, int> seats;
  QString myUsername;
signals:
  void avatarChange(quint8);
public slots:
  void possibleChangeAvatarLeft();
 void possibleChangeAvatarRight();
};

#endif
