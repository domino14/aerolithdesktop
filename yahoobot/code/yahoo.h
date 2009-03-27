
// programmed by C3sar
// credit goes to halcy0nical for helping me figure out the yahoo protocol
// before the encryption days
//
// i singlehandedly figured out the encryption, don't be a lamer and say i stole it
// from you

#ifndef _YAHOO_H_
#define _YAHOO_H_

using namespace std;
#include <iostream>
#include <string>
#include "PracticalSocket.h"
#include "Randomizer.h"
#include "crossword.h"
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include "trivia.h"
#include <inttypes.h>

typedef enum
  {
    FIRSTMSG, SENTY, GOTKEY, SENTJOINREQUEST, RECEIVED_ID, SENT_COOKIE, CONNECTED
  } connect_state;

#define RCVBUFSIZE 20000

#define LOGINSERVER "209.73.168.74"

#define SEND_KEY 1
#define RECEIVE_KEY 0




class player
{
 public:
  player(string playername2, int rating2)
    {
      playername = playername2;
      rating = rating2;
    }
 
  string playername;
  int rating;

};

class playerinfo
{
 private:

  vector <player> Players;
  vector <player>::iterator startIterator;

 public:
  void init()
    {
      Players.clear();
    }
  void addplayer(string playername, int rating)
    {
      Players.push_back(player(playername, rating) );
      cout << playername << " has rating " << rating << flush << endl;
    }
  void removeplayer(string playername)
    {
      startIterator = Players.begin();
      for (unsigned int i = 0; i < Players.size(); i++)
	if (Players[i].playername == playername)
	  {
	    Players.erase(startIterator + i);
	    return;
	  }
      cout << playername << " left." << endl;
    }
  void modifyplayer(string playername, int rating)
    {
      startIterator = Players.begin();
      for (unsigned int i = 0; i < Players.size(); i++)
	if (Players[i].playername == playername)
	  {
	    Players[i].rating = rating;
	    cout << playername << " has rating " << rating << endl;
	    return;
	  }
      addplayer(playername, rating);
    }


};
/*
struct table
{
  int tablenum;
  int timer, inctimer;
  int challengemode, numplayers, threemintimer, rated, privacy;
};

class tableinfo
{
 public:
  vector <struct table> Tables;
  vector <struct table>::iterator startIterator;  
  void addtable(int tablenum, int timer, int inctimer, int challengemode, 
		int numplayers, int threemintimer, int rated, int privacy)
    {
      Tables.push_back( {tablenum, timer, inctimer, challengemode, numplayers,
			   threemintimer, rated} );
      cout << "Table " << tablenum << ":  Timer = " << timer << "/" << inctimer <<
	"     Challenge = " << challengemode << "    3 min timer = " 
	   << threemintimer << "   Rated = " << rated << "  Privacy = " << 
	privacy << endl;
  
    }

  void removetable(int tablenum)
    {
      startIterator = Tables.begin();
      for (int i = 0; i < Tables.size(); i++)
	if (Tables[i].tablenum == tablenum)
	  {
	    Tables.erase(startIterator + i);
	    cout << "Table " << tablenum << " exists no more." << endl;
	    return;
	  }
    }
  void modifytable(int tablenum, int timer, int inctimer, int challengemode, 
		int numplayers, int threemintimer, int rated)
    {
      startIterator = Tables.begin();
      for (int i = 0; i < Tables.size(); i++)
	if (Tables[i].tablenum == tablenum)
	  {
	    Tables[i].timer = timer;
	    Tables[i].inctimer = inctimer;
	    Tables[i].challengemode = challengemode;
	    Tables[i].numplayers = numplayers;
	    Tables[i].threemintimer = threemintimer;
	    Tables[i].rated = rated;
	    cout << "Table " << tablenum << ":  Timer = " 
		 << timer << "/" << inctimer <<
	      "     Challenge = " << challengemode << "    3 min timer = " 
		 << threemintimer << "   Rated = " << rated << "  Privacy = " 
	      << privacy << endl;

	    return;
	  }

      addtable(tablenum, timer, inctimer, challengemode, 
	       numplayers, threemintimer, rated);

    }

  
};

*/



class yahoocmd
{
 public:
  unsigned char returnnextbyte(unsigned int b);

  yahoocmd()
    {
      mysendoffset = 0;
      myreceiveoffset = 0;
      state = FIRSTMSG;
    }

  void setbroadcast(int x);
  void init(string user, string pass, string cont, 
	    string gameroom, string server, Gamemode mygamemode);
  void pass(int table);
  int keygen(string yahookey);
  string encryptdata(string toencrypt, int sendorreceive);
  int getreply();
  int chatsay(string strtosay);
  int jointable(int tablenum);
  int sittable(int tablenum, int seat);
  int standtable(int tablenum);
  int leavetable(int tablenum);
  int pressstart(int tablenum);
  int printbytes(string mycmd);
  int ProcessCommand(string mycmd);
  int ExtractCommands(string mycmd);
  int chatsaytable(int tablenum, string strtosay);
  int fucktable(int tablenum);
  int ProcessReply(string reply, int len);
  string extractcookie (string mycookiestring);
  int obfuscate();
  int obfuscate(int table);
  int getcook();
  int start_connection(int listen);
  int sendpm(string username, string message);
  int createtable(int rated);
  int connect();
  int disconnect();
  void get_tiles(string mycmd, int firsttime);
  uint32_t get_rand(int i, int index);
  uint64_t get_seed(string mycmd);
  void pass_tiles(vector <char> passed_tiles, int table); 
  int find_in_player_tileset(char tiletofind);
  void get_new_tiles(int table);
  void clear_temprep();
  int makeplays(int table);
  void playtiles(int table);
  void placetile(const char letter, const int xboard, const int yboard, int table);
  int help_player(int table, int option);
  void exchangetiles(int table, string tilestoexchange);
  void get_table_parameters(string mycmd);
  string extract_def(string mystr);
  string define(string myword);
  void process_pm(string chatname, string chattext);
  void processtablecommand(string mycmd, int commandlength);
  string writeutf(string strtosay);


  int triviamode;

private:
  unsigned char receive_key[64];
  unsigned char send_key[64];
  int mysendoffset;
  int myreceiveoffset;
  connect_state state;
  int echomode, showims, anonmode, mathmode, broadcast, h4x0rmode; 
  int annoying;
  int curturn;
  string mycookie;
  string myid;
  string username;
  string password;
  string controller;
  string GAMEROOM;
  string GAMESERVER;
  TCPSocket client;
  rand_class myrand[2];       //0 activates at the beginning of the game
  // 1 is the "future" i.e. it activates right when the table is entered and 
  // predicts the next game's tiles
      //  int tile_probability[26];
  int N;
  int numplacedtiles;
  vector <char> playertiles[2];
  char temprep[15][15];
  int stillmyturn; // used when submitting a bad word 
  int litmode;
  generic_crossword_game mylit;
  int myseat; // seat where I am sitting. used for the literati bot
  Gamemode gamemode;
  int wordtoplay;
  string curplayer; // player playing me
  // tableinfo tableclass;
  playerinfo playerclass;
  int chatsayenabled;
  int startplaying;
  string packetheader;

  /// trivia variables:


};



#endif
