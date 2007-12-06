#ifndef _UNSCRAMBLE_GAME_TABLE_H_
#define _UNSCRAMBLE_GAME_TABLE_H_

#include <QtCore>
#include <QtGui>
#include "ui_tableForm.h"
#include "ui_playerInfoForm.h"

class UnscrambleGameTable : public QWidget
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
	void setAvatar(QString, quint8);
	void gotTimerValue(quint16 timerval);
	void gotWordListInfo(QString);
	void setMyUsername(QString);
private:
	QGraphicsScene gfxScene;
	Ui::tableForm tableUi;
	Ui::playerInfoForm playerUis[6];
	
	QList <QPixmap> tilesList;
	QList <QPixmap> chipsList; 

	QString myUsername;

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
