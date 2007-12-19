#ifndef _UNSCRAMBLE_GAME_TABLE_H_
#define _UNSCRAMBLE_GAME_TABLE_H_

#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "ui_tableForm.h"
#include "ui_playerInfoForm.h"
#include "ui_solutionsForm.h"
#include "ui_tableCustomizationForm.h"
#include "tile.h"
#include "chip.h"

#define NUM_AVATAR_IDS 73 

class GameTable : public QWidget
{

	Q_OBJECT
public:
	GameTable(QWidget* parent = 0, Qt::WindowFlags f = 0, int gamePlayers = 6);
	virtual ~GameTable() = 0;
	void setMyUsername(QString);

	void setAvatar(QString, quint8);
	virtual void setReadyIndicator(QString) = 0;
	virtual void clearReadyIndicators() = 0;
	virtual void setupForGameStart() = 0;
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
	UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f, QSqlDatabase wordDb);
	~UnscrambleGameTable();
	void resetTable(quint16, QString, QString);
	void leaveTable();
	void addPlayer(QString, bool);
	void removePlayer(QString, bool);
	void addPlayers(QStringList);

	void setupForGameStart();
	void gotChat(QString);
	void gotTimerValue(quint16 timerval);
	void gotWordListInfo(QString);

	void clearSolutionsDialog();
	void populateSolutionsTable();

	void addNewWord(int, QString, QStringList, quint8);
	void clearAllWordTiles();
	void answeredCorrectly(int index, QString username, QString answer);

	void setReadyIndicator(QString);
	void clearReadyIndicators();

private:
	QGraphicsScene gfxScene;
	Ui::tableForm tableUi;

	QSqlDatabase wordDb;
	QList <Tile*> tiles;
	QList <Chip*> chips;
	QList <Chip*> readyChips;
	QGraphicsPixmapItem* tableItem;
	QDialog* solutionsDialog;
	Ui::solutionsForm uiSolutions;

	QWidget* preferencesWidget;
	Ui::tableCustomizationForm uiPreferences;


	struct wordQuestion
	{
		wordQuestion(QString a, QStringList s, quint8 n)
		{
			alphagram = a;
			solutions = s;
			numNotYetSolved = n;
		};

		QString alphagram;
		QStringList solutions;
		quint8 numNotYetSolved;
		Chip* chip;
		QList <Tile*> tiles;
	};

	QList <wordQuestion> wordQuestions;
	QSet <QString> rightAnswers;
	double verticalVariation;
	double heightScale;
	void loadUserPreferences();
	void swapXPos(Tile*, Tile*);
	int getTileWidth(int wordLength);
	void getBasePosition(int index, double& x, double& y, int tileWidth);
protected:
	virtual void closeEvent(QCloseEvent*);
signals:
	void giveUp();
	void sendStartRequest();
	void guessSubmitted(QString);
	void chatTable(QString);
	void sendPM(QString);
	void exitThisTable();

	private slots:
		void enteredGuess();
		void enteredChat();
		void sendPM(QListWidgetItem* item);

		void alphagrammizeWords();
		void shuffleWords();
		void tileWasClicked();
		void setZoom(int);
		void changeTileColors(int);
		void changeFontColors(int);
		void changeTableStyle(int);
		void changeTileBorderStyle(bool);
		void changeVerticalVariation(bool);
		void changeBackground(int index);
		void changeTileAspectRatio(bool);
		void saveUserPreferences();


public:


};



#endif
