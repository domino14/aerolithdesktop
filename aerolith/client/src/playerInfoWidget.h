#ifndef _PLAYERINFOWIDGET_H_
#define _PLAYERINFOWIDGET_H_

#define NUM_AVATAR_IDS 73 
// 1-73

#include <QtGui>

class avatarLabel : public QLabel
{
Q_OBJECT

public:
	avatarLabel(QWidget* parent = 0);
signals:
	void leftMouseClicked();
	void rightMouseClicked();

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	bool left_pressed, right_pressed;
	bool inLabel(const QPoint &p);
};

class playerInfoWidget : public QWidget
{
Q_OBJECT
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
  void setMyUsername(QString username);
 private:
  QHBoxLayout* playerInfoLayout[6];
  QListWidget* playerLists[6];
  QLabel* playerNames[6];
  avatarLabel* playerAvatars[6];
  QLabel* playerStatus[6];
  QHash <QString, int> seats;
  QString myUsername;
signals:
	void avatarChange(quint8);
public slots:
	void possibleChangeAvatarLeft();
	void possibleChangeAvatarRight();
};

#endif
