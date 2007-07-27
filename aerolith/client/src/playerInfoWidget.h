#ifndef _PLAYERINFOWIDGET_H_
#define _PLAYERINFOWIDGET_H_

#include <QtGui>

class playerInfoWidget : public QWidget
{
 public:
  playerInfoWidget();
  void clearAndHide();
  void setupForGameStart();
  void answered(QString username, QString answer);
  void addPlayers(QStringList playerList);
  void addPlayer(QString player, bool gameStarted);
  void removePlayer(QString player, bool gameStarted);
  void leaveTable();
  void setReadyIndicator(QString username);
  void setAvatar(QString username, quint8 avatarID);
 private:
	 QHBoxLayout* playerInfoLayout[6];
  QListWidget* playerLists[6];
  QLabel* playerNames[6];
  QLabel* playerAvatars[6];
	QLabel* playerStatus[6];
  QHash <QString, int> seats;
};

#endif
