//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

#include <QtCore>

// this has definitions for client/server constants such as packet types

#define GAME_TYPE_UNSCRAMBLE 0
#define GAME_TYPE_WORDGRIDS 1
#define GAME_TYPE_BONUS 2
#define GAME_TYPE_BLANAGRAMS 3
#define GAME_TYPE_TAXESHOLDME 4
#define GAME_TYPE_ANAGRAMS 5


/* table 'cycle' type (marathon, cycle, random, daily challenge, etc) for Unscramble game */

#define TABLE_TYPE_DAILY_CHALLENGES 3
#define TABLE_TYPE_RANDOM_MODE 0
#define TABLE_TYPE_MARATHON_MODE 2 
#define TABLE_TYPE_CYCLE_MODE 1

/* list type for Unscramble game */
#define LIST_TYPE_INDEX_RANGE_BY_WORD_LENGTH 1
#define LIST_TYPE_NAMED_LIST 2
#define LIST_TYPE_USER_LIST 3
#define LIST_TYPE_ALL_WORD_LENGTH 4
#define LIST_TYPE_DAILY_CHALLENGE 5

/* game modes for unscramble game (user lists) */
#define UNSCRAMBLEGAME_USERLIST_MODE_CONTINUE 1
#define UNSCRAMBLEGAME_USERLIST_MODE_RESTART 2
#define UNSCRAMBLEGAME_USERLIST_MODE_FIRSTMISSED 3


/* network packet headers */

/*type byte*/
#define CLIENT_PONG '?'
#define CLIENT_CHAT_ACTION 'a'
#define CLIENT_LOGIN 'e'
#define CLIENT_CHAT 'c'
#define CLIENT_PM 'p'
#define CLIENT_NEW_TABLE 't'
#define CLIENT_VERSION 'v'
#define CLIENT_JOIN_TABLE 'j'
#define CLIENT_LEAVE_TABLE 'l'
#define CLIENT_TABLE_COMMAND '='
#define CLIENT_HIGH_SCORE_REQUEST 'h'
#define CLIENT_REGISTER 'r'
#define CLIENT_UNSCRAMBLEGAME_LIST_UPLOAD 'u'
#define CLIENT_UNSCRAMBLEGAME_LISTINFO_REQUEST 'q'
#define CLIENT_UNSCRAMBLEGAME_DELETE_LIST 'x'
#define CLIENT_SUGGESTION_OR_BUG_REPORT 'k'

// table subcommands
#define CLIENT_TABLE_READY_BEGIN 'b'
#define CLIENT_TABLE_CHAT 'c'
#define CLIENT_TABLE_GIVEUP 'u'
#define CLIENT_TABLE_ACTION 'a'
#define CLIENT_TRY_SITTING 's'
#define CLIENT_TRY_STANDING '-'
#define CLIENT_STAND_UP 'p'
#define CLIENT_TABLE_AVATAR 'i'
#define CLIENT_TABLE_PRIVACY 'y'
#define CLIENT_TABLE_INVITE 't'
#define CLIENT_TABLE_BOOT '!'
// misc table subcommands, by game
#define CLIENT_TABLE_UNSCRAMBLEGAME_CORRECT_ANSWER 'w'
#define CLIENT_TABLE_UNSCRAMBLEGAME_QUESTION_LIST 'q'
#define CLIENT_TABLE_UNSCRAMBLEGAME_SAVE_REQUEST 'd'


#define SERVER_PING '?'
#define SERVER_HIGH_SCORES 'H'
#define SERVER_LEFT_TABLE 'L'
#define SERVER_KILL_TABLE 'K'
#define SERVER_NEW_TABLE 'T'
#define SERVER_JOIN_TABLE 'J'
#define SERVER_PM 'P'
#define SERVER_WORD_LISTS 'W'
#define SERVER_LOGGED_IN 'E'
#define SERVER_ERROR '!'
#define SERVER_LOGGED_OUT 'X'
#define SERVER_MESSAGE 'S'
#define SERVER_CHAT 'C'
#define SERVER_TABLE_COMMAND '+'
#define SERVER_INVITE_TO_TABLE 'I'
#define SERVER_BOOT_FROM_TABLE 'B'
#define SERVER_MAX_BANDWIDTH '>'
#define SERVER_RESET_TODAYS_BANDWIDTH '&'

#define SERVER_UNSCRAMBLEGAME_LISTDATA_BEGIN '|'
#define SERVER_UNSCRAMBLEGAME_LISTDATA_CLEARONE '@'
#define SERVER_UNSCRAMBLEGAME_LISTDATA_ADDONE '#'
#define SERVER_UNSCRAMBLEGAME_LISTDATA_DONE '%'
#define SERVER_UNSCRAMBLEGAME_LISTSPACEUSAGE '*'
//table subcommands
#define SERVER_TABLE_CHAT 'C'
#define SERVER_TABLE_MESSAGE 'M'
#define SERVER_TABLE_READY_BEGIN 'B'
#define SERVER_TABLE_GAME_START 'S'
#define SERVER_TABLE_HOST 'H'
#define SERVER_TABLE_GAME_END 'E'
#define SERVER_TABLE_QUESTIONS 'Q'
#define SERVER_TABLE_TIMER_VALUE 'T'
#define SERVER_TABLE_NUM_QUESTIONS 'N'
#define SERVER_TABLE_GIVEUP 'U'
#define SERVER_TABLE_CORRECT_ANSWER 'Y'
#define SERVER_TABLE_SUCCESSFUL_SIT 'Z'
#define SERVER_TABLE_SUCCESSFUL_STAND 'P'
#define SERVER_TABLE_AVATAR_CHANGE 'I'
#define SERVER_TABLE_PRIVACY 'A'

// misc sucbommands by game
#define SERVER_TABLE_UNSCRAMBLEGAME_MAIN_QUIZ_DONE 'D' // the transition from current questions to loading missed questions
#define SERVER_TABLE_UNSCRAMBLEGAME_FULL_QUIZ_DONE 'F' // no more questions in missed or current
#define SERVER_TABLE_UNSCRAMBLEGAME_LIST_REQUEST 'R'
#define SERVER_TABLE_UNSCRAMBLEGAME_SAVING_ALLOWED 'W'

// other misc types
#define SERVER_WORD_LIST_REGULAR 'R'    // server sends client word list info on connect, this is one of the types
#define SERVER_WORD_LIST_CHALLENGE 'D'

void getUniqueRandomNumbers(QList<quint32>&numbers, quint32 start, quint32 end, int numNums);

// misc defines by game

// unscramble game
#define REMOTE_LISTSIZE_LIMIT 10000

//////////////
// other important defines
#define DEFAULT_PORT 2001


// utilities class
class Utilities
{
public:
    static QString getRootDir();
    static QString hexPrintable(const QByteArray x);

};


class PacketBuilder : public QObject
{
Q_OBJECT
public:
    PacketBuilder(QObject* parent);
    QByteArray getPacket();
private:
    QByteArray packet;
public:
    QDataStream o;
    void processForSending();
    void processRawPacketForSending(QByteArray);
    void resetPacket();

};


#endif
