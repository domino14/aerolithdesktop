#ifndef _UNSCRAMBLE_GAME_TABLE_H_
#define _UNSCRAMBLE_GAME_TABLE_H_

#include <QtCore>
#include <QtGui>
#include "ui_tableForm.h"
#include "ui_playerInfoForm.h"

#define NUM_AVATAR_IDS 73 

class GameTable : public QWidget
{

	Q_OBJECT
public:
	GameTable(QWidget* parent = 0, Qt::WindowFlags f = 0, int gamePlayers = 6);

	void setMyUsername(QString);

	void setAvatar(QString, quint8);
	void setReadyIndicator(QString);
	void setupForGameStart();
	void addToPlayerList(QString, QString);

signals:
	void avatarChange(quint8);
protected:
	QString myUsername;

	// most of these have to do with the player widgets.
	QHash <QString, int> seats;
	QList <Ui::playerInfoForm> playerUis;
	QList <QWidget*> playerWidgets;
	int numPlayers;
	void clearAndHidePlayers(bool hide);
	void playerLeaveTable();


	void addPlayersToWidgets(QStringList playerList);
	void removePlayerFromWidgets(QString, bool);
	void addPlayerToWidgets(QString, bool);

private slots:
		
	void possibleAvatarChangeLeft();
	void possibleAvatarChangeRight();
};

class UnscrambleGameTable : public GameTable
{
	Q_OBJECT

public:
	UnscrambleGameTable(QWidget* parent = 0, Qt::WindowFlags f = 0);
	void resetTable(quint16, QString, QString);
	void leaveTable();
	void addPlayer(QString, bool);
	void removePlayer(QString, bool);
	void addPlayers(QStringList);

	void gotChat(QString);
	void gotTimerValue(quint16 timerval);
	void gotWordListInfo(QString);
	
private:
	QGraphicsScene gfxScene;
	Ui::tableForm tableUi;

	
	QList <QPixmap> tilesList;
	QList <QPixmap> chipsList; 

	

protected:
	virtual void closeEvent(QCloseEvent*);
signals:
	void giveUp();
	void sendStartRequest();
	void avatarChange(quint8);
	void guessSubmitted(QString);
	void chatTable(QString);
	void exitThisTable();

	void shouldShowSolutions();

	private slots:
		void enteredGuess();
		void enteredChat();
		void sendPM(QListWidgetItem* item);

public:


};



#endif
