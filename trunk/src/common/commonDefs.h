#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

// this has definitions for client/server constants such as packet types

/* table 'cycle' type (marathon, cycle, random, daily challenge, etc) */

#define TABLE_TYPE_DAILY_CHALLENGES 3
#define TABLE_TYPE_RANDOM_MODE 0
#define TABLE_TYPE_MARATHON_MODE 2 
#define TABLE_TYPE_CYCLE_MODE 1


/* network packet headers */

/*type byte*/
#define CLIENT_PONG '?'
#define CLIENT_CHAT_ACTION 'a'
#define CLIENT_LOGIN 'e'
#define CLIENT_CHAT 'c'
#define CLIENT_PM 'p'
#define CLIENT_NEW_TABLE 't'
#define CLIENT_AVATAR 'i'
#define CLIENT_VERSION 'v'
#define CLIENT_JOIN_TABLE 'j'
#define CLIENT_LEAVE_TABLE 'l'
#define CLIENT_TABLE_COMMAND '='
#define CLIENT_HIGH_SCORE_REQUEST 'h'
#define CLIENT_REGISTER 'r'
// table subcommands
#define CLIENT_TABLE_READY_BEGIN 'b'
#define CLIENT_TABLE_GUESS 's'
#define CLIENT_TABLE_CHAT 'c'
#define CLIENT_TABLE_GIVEUP 'u'
#define CLIENT_TABLE_ACTION 'a'

#define SERVER_PING '?'
#define SERVER_HIGH_SCORES 'H'
#define SERVER_LEFT_TABLE 'L'
#define SERVER_KILL_TABLE 'K'
#define SERVER_NEW_TABLE 'T'
#define SERVER_JOIN_TABLE 'J'
#define SERVER_AVATAR_CHANGE 'I'
#define SERVER_PM 'P'
#define SERVER_WORD_LISTS 'W'
#define SERVER_LOGGED_IN 'E'
#define SERVER_ERROR '!'
#define SERVER_LOGGED_OUT 'X'
#define SERVER_MESSAGE 'S'
#define SERVER_CHAT 'C'
#define SERVER_TABLE_COMMAND '+'
//table subcommands
#define SERVER_TABLE_CHAT 'C'
#define SERVER_TABLE_MESSAGE 'M'
#define SERVER_TABLE_READY_BEGIN 'B'
#define SERVER_TABLE_GAME_START 'S'
#define SERVER_TABLE_GAME_END 'E'
#define SERVER_TABLE_ALPHAGRAMS 'W'
#define SERVER_TABLE_TIMER_VALUE 'T'
#define SERVER_TABLE_NUM_QUESTIONS 'N'
#define SERVER_TABLE_GIVEUP 'U'
#define SERVER_TABLE_GUESS_RIGHT 'A'




#endif
