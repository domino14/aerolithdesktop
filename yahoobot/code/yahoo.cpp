#include "yahoo.h"
#include "trivia.h"
#include <algorithm>
#include <inttypes.h>

int zero = 0;
extern Trivia trivia;

#define CHR(x) ( (unsigned char) (zero + x))
#define U_CHINT(x) ( (unsigned int)(unsigned char)x)
#define vbcrlf "\xD\xA\0"

char cesupper(char inputletter)
{
  if (inputletter >= 'a' && inputletter <= 'z') return inputletter - 'a' + 'A';
  if (inputletter >= 'A' && inputletter <= 'Z') return inputletter;
  if (inputletter >= '0' && inputletter <= '9') return inputletter;
  return ' ';
}

string removespaces(string input)
{
  string output = "";
  for (unsigned int i = 0; i < input.length(); i++)
    {
      if (input[i] != ' ') output+= input[i];
    }

   return output;

}

int cmp_answers(string s1, string s2)
{
  // returns 1 if equal
  transform(s1.begin(), s1.end(), s1.begin(), cesupper);
  transform(s2.begin(), s2.end(), s2.begin(), cesupper);
 

   // remove spaces
  
  

  if (removespaces(s1) == removespaces(s2)) return 1;
  return 0;
}



void yahoocmd::setbroadcast(int x)
{
  broadcast = x;
}

void yahoocmd::init(string user, string pass, string cont, 
		    string gameroom, string server, Gamemode mygamemode)
{
  mysendoffset = 0;
  myreceiveoffset = 0;
  echomode = 0; showims = 1; anonmode = 1; mathmode = 0; h4x0rmode = 0; 
  triviamode = 0; annoying = 0;
  state = FIRSTMSG;
  username = user; password = pass; controller = cont;
  GAMEROOM = gameroom;
  //GAMESERVER = "66.218.68.247";
  GAMESERVER = server;
  //      tile_probability
  N = 0;
  myseat = 0;
  mylit.initall();
  gamemode = mygamemode;
  litmode = gamemode.lit;
  playerclass.init();
  srand(time(NULL));
  chatsayenabled = 0;
  
}

int yahoocmd::connect()
{
  int bytesReceived;
  int i;
  string repstring;
  // just starts a connection and returns, that's it
  if (mycookie == "")
    {
      if (getcook() == 0) return 0;

    }
  try
    {
      client.connect(GAMESERVER, 11999);
      char reply[RCVBUFSIZE + 1];
      while (1)
	{
	  try
	    {
	      if (state == CONNECTED) break;
	      bytesReceived = client.recv(reply, RCVBUFSIZE);
	
	      reply[bytesReceived] = '\0';
	      repstring.resize(bytesReceived);
	      for (i = 0; i < bytesReceived; i++)
		repstring[i] = reply[i];
	      ProcessReply(repstring, bytesReceived);
	    }
	  catch (SocketException& e)
	    {
	      cout << "EXCEPTION " << e.what() << endl;
	    }
	}
    }
  catch (SocketException& e)
    {
      cout << "excpt " << e.what() << endl;
    }
  return 0;
}

int yahoocmd::disconnect()
{
  // disconnects
  client.discSocket();
  state = FIRSTMSG;
  mysendoffset = 0;
  myreceiveoffset = 0;
  return 0;
}

int yahoocmd::start_connection(int listen)
{
  int bytesReceived;
  string repstring;
  int i;
  if (getcook() == 0) 
    {
      cout << "Error getting cookie.\n";
      return 0;
    }

  try
    {
      string requeststring;
      char gamesreply[RCVBUFSIZE + 1];
      TCPSocket games_socket("66.218.71.230", 80); // games.yahoo.com
      requeststring = "GET /games/applet.html?room=";
      requeststring += "literati_int_c";  // TODO make dependent on GAMEROOM variable.
      requeststring += "&prof_id=chat_pf_1&small=no&follow=&nosignedcab=yes HTTP/1.1\r\n";

      requeststring += "Host: games.yahoo.com\r\n";

      requeststring += "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n";

      requeststring += "Accept: application/x-shockwave-flash, */*\r\n";
      requeststring += "Accept-Language: en-us\r\n";


      requeststring += "Connection: Keep-Alive\r\n";
      requeststring += "Cookie: ";
      requeststring += mycookie;
      requeststring += "\r\n";
      requeststring += "\r\n";
      games_socket.send( requeststring.c_str(), (int)requeststring.length());
      cout << "Sent " << requeststring << endl;
      
      if ((bytesReceived = (games_socket.recv(gamesreply, RCVBUFSIZE))) <= 0) 
	{
	  cerr << "Unable to read";
	  exit(1);
	}

    }
  catch (SocketException& e)
    {
      cout << e.what() << endl;
    }

  try
    {
      client.connect(GAMESERVER, 11999);
      char reply[RCVBUFSIZE + 1];
      cout << endl;
      cout << "Connecting to " << GAMEROOM << " with id " << username
	   << " and controller " << controller << endl;
      cout << endl;      
      while (1)
	{
	  try
	    {
	      if (listen == 0)
		if (state == CONNECTED) break;
	      
	      bytesReceived = client.recv(reply, RCVBUFSIZE);
	      repstring.resize(bytesReceived);
	      for (i = 0; i < bytesReceived; i++)
		repstring[i] = reply[i];
	      ProcessReply(repstring, bytesReceived);

	    }
	  catch (SocketException& e) 
	    {
	      cout << "EXCEPTION " << e.what() <<  endl;
	    }
	}
    }
  catch (SocketException& e)
    {
      cout << "Exception was caught: " << e.what() << endl;
    }

  return 1;
}

unsigned char yahoocmd::returnnextbyte(unsigned int b)
{
  return  (b*83);
}

int yahoocmd::keygen(string yahookey)
{
  int b = yahookey[7]; // the 8th byte
  int i;
  receive_key[0] = returnnextbyte((unsigned int)b);

  for (i = 1; i <= 63; i++)
    receive_key[i] = returnnextbyte((unsigned int)receive_key[i - 1]);

  b = yahookey[3];
  send_key[0] = returnnextbyte((unsigned int)b);

  for (i = 1; i <= 63; i++)
    send_key[i] = returnnextbyte((unsigned int)send_key[i - 1]);

  return 1;
}

string yahoocmd::encryptdata(string toencrypt, int sendorreceive)
{
  int a;
  int byteinxor;
  string encrypteddata = "";
  int enclength = toencrypt.length();

  if (sendorreceive == SEND_KEY)
    {
      for (a = 0; a < enclength; a++)
	{
	  byteinxor = a + mysendoffset;
	  if (byteinxor > 63) byteinxor %= 64;
	  encrypteddata = encrypteddata + 
	    (char) (toencrypt[a] ^ send_key[byteinxor]); // bitwise XOR
	}
      mysendoffset = (mysendoffset + enclength) % 64;

    }

  if (sendorreceive == RECEIVE_KEY)
    {
      for (a = 0; a < enclength; a++)
        {
          byteinxor = a + myreceiveoffset;
          if (byteinxor > 63) byteinxor %= 64;
          encrypteddata = encrypteddata + 
	    (char) (toencrypt[a] ^ receive_key[byteinxor]); // bitwise XOR      
	}
      myreceiveoffset = (myreceiveoffset + enclength) % 64;
    }
  return encrypteddata;
}

string utf_ize(string mystr)
{
  unsigned int i = 0;
  unsigned char mybytemask1 = 128+64;
  unsigned char mybytemask2 = 128;
  unsigned char mybyte;
  unsigned char firstbyte, secondbyte;
  string tempstr;
  while (i < mystr.length())
    {
      if ( (unsigned char)mystr.at(i) > (unsigned char)127)
	{
	  mybyte = mystr.at(i);
	  secondbyte = mybytemask2 + (mybyte & 0x3F);
	  tempstr = secondbyte;
	  firstbyte = mybytemask1 + (mybyte >> 6);
	  mystr.at(i) = firstbyte;
	  i++;
	  mystr.insert(i, tempstr);
	}
      i++;
    }
  

  return mystr;

}
string mapstr(string strtosay)
{
  for (unsigned int i = 0; i < strtosay.length(); i++)
    {
      strtosay[i] = tolower(strtosay[i]);
      switch (strtosay[i])
	{
	case 'a':
	  strtosay[i] = 0xe2;
	  break;
	case 'e':
	  strtosay[i] = 0xe9;
	  break;
	case 'i':
	  strtosay[i] = 0xee;
	  break;
	case 'o':
	  strtosay[i] = 0xf8;
	  break;
	case 'u':
	  strtosay[i] = 0xf9;
	  break;
	case 'c':
	  strtosay[i] = 0xc7;
	  break;
	case 's':
	  strtosay[i] = 0xa7;
	  break;
	case 'y':
	  strtosay[i] = 0xa5;
	  break;
	case 'l':
	  strtosay[i] = 0xa3;
	  break;
	case 'x':
	  strtosay[i] = 0xd7;
	  break;
	case 'n':
	  strtosay[i] = 0xf1;
	  break;
	case 'd':
	  strtosay[i] = 0xd0;
	  break;
	case 'b':
	  strtosay[i] = 0xdf;
	  break;
	case 'r':
	  strtosay[i] = 0xae;
	  break;
	case 'p':
	  strtosay[i] = 0xfe;
	  break;
	case '.':
	  strtosay[i] = 0xb7;
	  break;
	case ',':
	  strtosay[i] = 0xb8;
	  break;
	case 't':
	  strtosay[i] = '7';
	  break;
	case '?':
	  strtosay[i] = 0xbf;
	  break;
	case 'g':
	  strtosay[i] = '9';
	  break;
	case 'h':
	  strtosay[i] = 0xb5;
	  break;
	}
    }
  return strtosay;
}

string yahoocmd::writeutf(string strtosay)
{
  // write a UTF string
  // just take the string and add the two bytes before it for length

  int mylength = strtosay.size();
  int hb = (mylength & 65280) >> 8;
  int lb = (mylength & 255);
  string retstr (1, CHR(hb));
  retstr += CHR(lb);
  retstr += strtosay;
  return retstr;

}


int yahoocmd::chatsay(string strtosay)
{
  if (chatsayenabled == 0) return 0;
  string cmdtosend;
  if (h4x0rmode) strtosay = mapstr(strtosay);
  strtosay = utf_ize(strtosay);

  string functioncode = "C";
  cmdtosend = packetheader + writeutf(functioncode + writeutf(strtosay));
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());
  
  return 1;
}

int yahoocmd::sendpm(string username, string message)
{
  string cmdtosend;
  string functioncode = "#";
  
  cmdtosend = packetheader + writeutf(functioncode + writeutf(username) + writeutf(message));
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::createtable(int rated)
{
  string cmdtosend;
  cmdtosend = "d";
  cmdtosend += CHR(0);
  cmdtosend += myid;
  cmdtosend += CHR(0);
  if (rated == 0)
    {
      cmdtosend += CHR(17);
      cmdtosend += "N";
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += "bd";
      cmdtosend += CHR(0);
      cmdtosend += CHR(1);
      cmdtosend += "1";
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += "np";
      cmdtosend += CHR(0);
      cmdtosend += CHR(1);
      cmdtosend += "2";
    }
  else if (rated == 1)
    {
      cmdtosend += CHR(23);
      cmdtosend += "N";
      cmdtosend += CHR(0);
      cmdtosend += CHR(3);
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += "bd";
      cmdtosend += CHR(0);
      cmdtosend += CHR(1);
      cmdtosend += "1";
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += "rd";
      cmdtosend += CHR(0);
      cmdtosend += CHR(0);
      cmdtosend += CHR(0);
      cmdtosend += CHR(2);
      cmdtosend += "np";
      cmdtosend += CHR(0);
      cmdtosend += CHR(1);
      cmdtosend += "2";
    }
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());
  return 0;
}

int yahoocmd::chatsaytable(int tablenum, string strtosay)
{
  strtosay = utf_ize(strtosay); 
  
  
  string cmdtosend;
  string functioncode = "+";
  string functioncode2 = "C";
  string tablestring(1, CHR(tablenum));

  cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2 + writeutf(strtosay));
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());
  return 1;
}

int yahoocmd::jointable(int tablenum)
{
  string cmdtosend;
  string functioncode = "J";
  string tablestring(1, CHR(tablenum));
  cmdtosend = packetheader + writeutf(functioncode + tablestring); 
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::sittable(int tablenum, int seat)
{
  string cmdtosend;
  string functioncode = "+";
  string functioncode2 = "T";
  string tablestring(1, CHR(tablenum));
  string seatstring(1, CHR(seat));

  cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2 + seatstring);
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::standtable(int tablenum)
{
  string cmdtosend;
  string functioncode = "+";
  string functioncode2 = "D";
  string tablestring(1, CHR(tablenum));

  cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2);
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::leavetable(int tablenum)
{
  string cmdtosend;
  string functioncode = "L";
  string tablestring(1, CHR(tablenum));

  cmdtosend = packetheader + writeutf(functioncode + tablestring);
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::fucktable(int tablenum)
{
  int i;
  for (i = 0; i < 10000; i++)
    {
      jointable(tablenum);
      chatsaytable(tablenum, "    STOP FUCKING WITH ME    ");
      leavetable(tablenum);
    }
  return 1;
}

int yahoocmd::pressstart(int tablenum)
{
  string cmdtosend;
  string functioncode = "+";
  string functioncode2 = "B";
  string tablestring(1, CHR(tablenum));

  cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2);
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());

  return 1;
}

int yahoocmd::printbytes(string mycmd)
{
  cout << "PACKET (LENGTH " << mycmd.size() << ") " << endl;
  for (unsigned int a = 0; a < mycmd.size(); a++)
    {
      cout << uint16_t(mycmd[a]) << " ";
    }
  cout << endl;
  cout << mycmd << endl;
  return 1;
}

int yahoocmd::find_in_player_tileset(char tiletofind)
{
  if (tiletofind == 32) tiletofind = '?';
  if (tiletofind >= 'a' && tiletofind <= 'z') tiletofind = '?';

  for (unsigned int i = 0; i < playertiles[curturn].size(); i++)
    if (playertiles[curturn][i] == tiletofind)
      return i;
  
  return -1;
}

void yahoocmd::pass_tiles(vector <char> passed_tiles, int table)
{
  
  cout << "Passed tiles: ";

  for (unsigned int i1 = 0; i1 < passed_tiles.size(); i1++)
    {
      int j1 = find_in_player_tileset(passed_tiles[i1]);
      if (passed_tiles[i1] != 32) cout << passed_tiles[i1];
      else cout << "?";
      playertiles[curturn].erase(playertiles[curturn].begin()+j1, 
				 playertiles[curturn].begin()+j1 + 1);

      if (passed_tiles[i1] != 32)
	mylit.tiles.insert(mylit.tiles.size() != 0 ? mylit.tiles.begin()+get_rand((int)mylit.tiles.size(), 0) : mylit.tiles.begin(), 1, passed_tiles[i1]);
      else
	mylit.tiles.insert(mylit.tiles.size() != 0 ? mylit.tiles.begin()+get_rand((int)mylit.tiles.size(), 0) : mylit.tiles.begin(), 1, '?');
    }
  cout << endl;
  get_new_tiles(table);
}

void yahoocmd::get_new_tiles(int table)
{
  // basically, dequeue from tiles until we either have 7 tiles or there are no more
  // tiles
  char mytile;
  for ( ; playertiles[curturn].size() < 7 && mylit.tiles.size() > 0; )
    {
      mytile = mylit.tiles[mylit.tiles.size()-1];
      mylit.tiles.pop_back();
      playertiles[curturn].push_back(mytile);
    }
  // cout new tileset
  cout << "Player " << curturn << "'s new tiles: " << endl;
  if (playertiles[curturn].size() == 0)
    {
      cout << "GAME OVER" << endl;
      chatsaytable(table, "gg ty");
      if (gamemode.continuous == 1) 
	{
	  pressstart(table);
	  system("sleep 1");
	}
    }
  else
    {
      for (unsigned int i = 0; i < playertiles[curturn].size(); i++)
	{
	  cout << playertiles[curturn][i];
	}
      cout << endl;
    }
}

void yahoocmd::clear_temprep()
{
  int i, j;
  for (i = 0; i < 15; i++)
    for (j = 0; j < 15; j++)
      temprep[i][j] = -1;

}

int yahoocmd::help_player(int table, int option)
{
  
  //gameboard should already be known
  if(option == 0)
    {
      // best move

      int maxindex;
      for (int i = 0; i < 27; i++)
	mylit.tilerack[i] = 0;
      for (unsigned int i = 0; i < playertiles[curturn].size(); i++)
	{
	  if (playertiles[curturn][i] >= 'A' && playertiles[curturn][i] <= 'Z')
	    mylit.tilerack[playertiles[curturn][i] - 'A'] += 1;
	  if (playertiles[curturn][i] == '?')
	    mylit.tilerack[26] += 1;
	}
      mylit.clearmovearray();
      mylit.generateallplays();
      maxindex = mylit.sortmoves();
      if (maxindex >= 0)
	{
	  string buf;
	  buf = "You can play " + mylit.highscoremoves.at(maxindex).word;
	  char score[10];
	  if (mylit.highscoremoves.at(maxindex).strplay[0] >= 'a')
	    {
	      buf += " vertically starting at row ";
	      sprintf(score, "%d", mylit.highscoremoves[maxindex].strplay[1]-'A'+1);
	      buf += score;	      
	      buf += " and column ";
	      sprintf(score, "%d", mylit.highscoremoves[maxindex].strplay[0]-'a'+1);
	      buf += score;	      
	    }
	  else
	    {
	      buf += " horizontally starting at row ";
	      sprintf(score, "%d", mylit.highscoremoves[maxindex].strplay[0]-'A'+1);
	      buf += score;
	      buf += " and column ";
	      sprintf(score, "%d", mylit.highscoremoves[maxindex].strplay[1]-'a'+1);
	      buf += score;;

	    }
	  chatsaytable(table, buf);
	}
      else
	{
	  chatsaytable(table, "I can't find a move for you.");
	 
	}
    }
  if (option == 1)
    {
      string buf = "";
      for (int i = (int)mylit.tiles.size() - 1; i >= 0; i--)
	buf = buf + mylit.tiles.at(i); // backwards order
      chatsaytable(table, buf);
      buf = "";
      for (unsigned int i = 0; i < playertiles[myseat].size(); i++)
	buf += playertiles[myseat].at(i);
      chatsaytable (table, "My tiles are: " + buf);
    }

  return 0;
}

int yahoocmd::makeplays(int table)
{
  // first we must determine what the board is, and what our tiles are.
  // PUT THE BOARD in mylit.gameboard
  // put the tile rack in mylit.tilerack
  // call generateallplays

  // since we have a temporary board, we should "add" it to our gameboard now
  int i;
  if (gamemode.rated == 1)
    if (startplaying == 0)
      {
	startplaying = 1;
	cout << "BLAH: ";
	cin >> i;
      }

  for (int j = 0; j < 15; j++)
    for (int i = 0; i < 15; i++)
      if (temprep[i][j] != -1) mylit.gameboard[j][i] = temprep[i][j];
  // now put our tile rack in mylit.tilerack
  for (int i = 0; i < 27; i++)
    mylit.tilerack[i] = 0;
  for (unsigned int i = 0; i < playertiles[curturn].size(); i++)
    {
      if (playertiles[curturn][i] >= 'A' && playertiles[curturn][i] <= 'Z')
	mylit.tilerack[playertiles[curturn][i] - 'A'] += 1;
      if (playertiles[curturn][i] == '?')
	mylit.tilerack[26] += 1;
    }
  mylit.clearmovearray();
  /*  for (int j = 0; j < 15; j++)
    {
      for (int i = 0; i < 15; i++)
	{
	  cout << mylit.gameboard[i][j];
	}
      cout << endl;
    }
  */
  //mylit.printgameboard();
  mylit.printtilerack();



  mylit.generateallplays();
  playtiles(table);
  return 1;
}

void yahoocmd::playtiles(int table)
{
  // criterion: best scoring play
  //for (unsigned int i = 0; i < mylit.highscoremoves.size(); i++)
  //if (mylit.mytrie.findword(mylit.highscoremoves.at(i).word) == 0)
  //  cout << "BAD " << mylit.highscoremoves.at(i).word << endl;

  int maxindex = 0;
  int direction;
  int startx, starty;
  int x, y;
  unsigned int letter;
  string buf;
  int ipass = 0, iexchange = 0;
  maxindex = mylit.sortmoves();

  unsigned int wordlen;
  if (gamemode.lowball == 0) maxindex -= wordtoplay;
  else
    {
      if ((unsigned int)maxindex < 0 || 
	  (unsigned int)wordtoplay > mylit.highscoremoves.size()-1) ipass = 1;
      else maxindex = wordtoplay;
  
    }
  if (maxindex < 0) ipass = 1; // this means teh array is empty. always pass
  else
    {
      if (mylit.highscoremoves[maxindex].weightedscore < 10 && mylit.tiles.size() > 0)
	{
	  // we can exchange
	  iexchange = 1;
	}
      else
	{
	  mylit.highscoremoves[maxindex].printmove();
	  wordlen = mylit.highscoremoves[maxindex].word.length();
	}
  //cur_state = STATE_IDLE;
    }
  // Gg vertical  bB horizontal                                                        
  if (ipass)
    {
      // click pass!
      pass(table);
 
      if (gamemode.lowball == 1)
	{
	  chatsaytable(table, "You win this game.");
	  
	  // should resign
	}	
    }
  else if (!iexchange)
    {

      if (mylit.highscoremoves[maxindex].strplay[0] <= 'Z')
        direction = GOING_DOWN;
      else
        direction = GOING_ACROSS;

      if (direction == GOING_DOWN)
        {
          startx = mylit.highscoremoves[maxindex].strplay[0] - 'A';
          starty = mylit.highscoremoves[maxindex].strplay[1] - 'a';
        }
      if (direction == GOING_ACROSS)
        {
          startx = mylit.highscoremoves[maxindex].strplay[1] - 'A';
          starty = mylit.highscoremoves[maxindex].strplay[0] - 'a';
        }
      for (x = startx, y = starty, letter = 0; 
	   letter < wordlen; 
	   x+= direction, y+= !(direction), letter++)
        {
          if (mylit.highscoremoves[maxindex].word[letter] != mylit.gameboard[x][y])
	    placetile(mylit.highscoremoves[maxindex].word[letter], y, x, table);
	

        } // end for                                                                  


      /* click submit*/
       string cmdtosend;
       string functioncode = "+";
       string tablestring(1, CHR(table));
       string functioncode2 = "F";

       cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2);
       cmdtosend = encryptdata(cmdtosend, SEND_KEY);
       client.send(cmdtosend.c_str(), cmdtosend.size());

    }

  else if (iexchange)
    {
      cout << "Exchange tiles!" << endl;
      string myrack = mylit.highscoremoves[maxindex].rack;
      // determine whether we have an overflow of vowels or consonants.
      assert (myrack.length() == 7); // better fucking be 7
      int numvowels = 0, numconsonants = 0;
      string vowelrack, consonantrack;
      string tilestoexchange;
      for (unsigned int i = 0; i < myrack.size(); i++)
	{
	  if (myrack.at(i) == 'A' || myrack.at(i) == 'E' || myrack.at(i) == 'I' ||
	      myrack.at(i) == 'O' || myrack.at(i) == 'U')
	  {
	    numvowels++;
	    vowelrack += myrack.at(i);
	  }
	  else if ( (myrack.at(i) >= 'a' && myrack.at(i) <= 'z') || 
		    myrack.at(i) == '?')
	    ; // nothing. never pass blanks
	  else 
	    {
	      numconsonants++;
	      consonantrack+= myrack.at(i);
	    }
	}
      if ( numvowels - numconsonants > 5)
	{
	  // exchange at most 5 vowels
	  tilestoexchange = vowelrack.substr(0, 5);
	}
      else if (numconsonants - numvowels > 5)
	{
	  // exchange at most 5 consonants
	  tilestoexchange = consonantrack.substr(0, 5);
	}
      else
	{
	  // exchange all but blank
	  tilestoexchange = vowelrack + consonantrack;
	}
      exchangetiles(table, tilestoexchange);
    }
  else 
    {
      cout << "wtf" << endl;
    }
}

void yahoocmd::pass(int table)
{
  string cmdtosend;
  string functioncode = "+";
  string tablestring(1, CHR(table));
  string functioncode2 = "S";
  string fourzeros(4, CHR(0)); 

  cmdtosend = packetheader + writeutf(functioncode + tablestring + functioncode2 + fourzeros);
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());
  cout << "This function has not been tested. Remove if verify working." << endl;
}

void yahoocmd::exchangetiles(int table, string tilestoexchange)
{
  cout << "Exchange " << tilestoexchange << endl;
  string cmdtosend;
  cmdtosend = packetheader;
  cmdtosend += CHR(0);
  cmdtosend += CHR(7 + 4 * tilestoexchange.length());
  cmdtosend += "+";
  cmdtosend += CHR(table);
  cmdtosend += "S";
  cmdtosend += CHR(0);
  cmdtosend += CHR(0);
  cmdtosend += CHR(0);
  cmdtosend += CHR(tilestoexchange.length());
  for (unsigned int i = 0; i < tilestoexchange.length(); i++)
    {
      cmdtosend += CHR(0);
      cmdtosend += CHR(0);
      cmdtosend += CHR(0);
      cmdtosend += CHR(tilestoexchange.at(i));
    }

  //100 0 0 40 88 0 7 43 28 83 0 0 0 0	// pass nothing 
  cmdtosend = encryptdata(cmdtosend, SEND_KEY);
  client.send(cmdtosend.c_str(), cmdtosend.size());
}

void yahoocmd::placetile(const char letter, const int xboard, const int yboard, int table)
{
  // the protocol always takes COLUMN before ROW
  //                                x          y

  string cmdtosend;
  cmdtosend = packetheader;
  cmdtosend += CHR(0);
  cmdtosend += CHR(6);
  cmdtosend += "+";
  cmdtosend += CHR(table);
  cmdtosend += "P";

  if (letter >= 'a' && letter <= 'z')
    {
      //place blank
      cmdtosend += "4";
      cmdtosend += CHR(xboard);
      cmdtosend += CHR(yboard);
      cmdtosend = encryptdata(cmdtosend, SEND_KEY);
      //  cout << "Placed " << letter << endl;
      client.send(cmdtosend.c_str(), cmdtosend.size());
      
      // write in blank
      cmdtosend = packetheader;
      cmdtosend += CHR(0);
      cmdtosend += CHR(6);
      cmdtosend += "+";
      cmdtosend += CHR(table);
      cmdtosend += "H";
      cmdtosend += CHR(xboard);
      cmdtosend += CHR(yboard);
      cmdtosend += CHR(letter - 'a' + 26);
      cmdtosend = encryptdata(cmdtosend, SEND_KEY);
      //cout << "Placed " << letter << endl;
      client.send(cmdtosend.c_str(), cmdtosend.size());
    }
  if (letter >= 'A' && letter <= 'Z')
    {
      cmdtosend += letter - 'A';
      cmdtosend += CHR(xboard);
      cmdtosend += CHR(yboard);
      cmdtosend = encryptdata(cmdtosend, SEND_KEY);
      //cout << "Placed " << letter << endl;
      client.send(cmdtosend.c_str(), cmdtosend.size());


    }

}
int yahoocmd::obfuscate()
{
  string chattext;
  //  for (int j = 0; j < 8; j++)
  //for (int i = 128; i < 145; i++) 
  //  chattext += CHR(i);
  
  for (int i = 128; i < 255; i++)
    chattext += CHR(i);
  
  for (int i = 155; i < 255; i++)
    chattext += CHR( rand()%100+ 155);

  chatsay(chattext);

  return 1;
}

int yahoocmd::obfuscate(int table)
{
  string chattext;
  //  for (int j = 0; j < 8; j++)
  //for (int i = 128; i < 145; i++) 
  //  chattext += CHR(i);
  
  for (int i = 128; i < 255; i++)
    chattext += CHR(i);
  
  for (int i = 155; i < 255; i++)
    chattext += CHR( rand()%100+ 155);

  chatsaytable(table, chattext);
  return 1;

}

void yahoocmd::process_pm(string chatname, string chattext)
{
  if (showims == 1 && chattext.size() < 50) 
    cout << "INSTANT MESSAGE FROM " << chatname << ": " << chattext << endl;
  if (showims == 1 && chattext.size() >= 50)
    cout << "possible spam IM\t";
  if (chatname == controller && chattext == "e")
    echomode = !echomode;
  if (chatname == controller && chattext == "b")
    showims = !showims;
  if (chattext[0] == 's' && anonmode == 1 && chatname != controller)
    {
      chatsay("### " + 
	      chattext.substr(1, chattext.size() - 1));
      cerr << chatname << " sent this message. " << endl;
    }
  if (chattext[0] == 's' && chatname == controller)
    chatsay(chattext.substr(1, chattext.size() - 1));
  if (chatname == controller && chattext == "a")
    anonmode = !anonmode;
  if (chatname == controller && chattext == "d")
    broadcast = !broadcast;
  if (chatname == controller && chattext == "m")
    mathmode = !mathmode;
  if (chatname == controller && chattext == "t")
    {
      if (triviamode == 1)
	{
	  chatsay("    TRIVIA BOT IS GOING TEMPORARILY OFFLINE FOR RECOMPILATION");
	  alarm(0);
	  
	}
      triviamode = 0;
    }      
      
  if (chatname == controller && chattext == "h")
    h4x0rmode = !h4x0rmode;
  if (chatname == controller && chattext[0] == 'j')
    jointable( (chattext[1]-'0')*10 + chattext[2]-'0');
  if (chatname == controller && chattext[0] == 'z')
    sittable( (chattext[1]-'0') * 10 + chattext[2]-'0', chattext[3] -'0');
  if (chatname == controller && chattext[0] == 'B')
    pressstart( (chattext[1]-'0') * 10 + chattext[2]-'0');
  if (chatname == controller && chattext[0] == 'S')
    chatsaytable( (chattext[1]-'0')* 10 + chattext[2]-'0', 
		  chattext.substr(3, chattext.size()-3));
  if (chatname == controller && chattext[0] == 'L')
    leavetable( (chattext[1]-'0')*10 + chattext[2]-'0');
  if (chatname == controller && chattext[0] == 'f')
    fucktable ( (chattext[1]-'0')*10 + chattext[2]-'0');
  if (chatname == controller && chattext[0] == 'c')
    createtable( (chattext[1]-'0'));
  if (chatname == controller && chattext == "lit")
    litmode = !litmode;
  if (chatname == controller && chattext == "start")
    startplaying = 1;
  if (chattext[0] == '*') 
    obfuscate();
  if (chattext[0] == '!')
    obfuscate( (chattext[1]-'0')*10 + chattext[2]-'0');
  /// if (chattext == "cookie")
  //chatsay("/open " + mycookie);
  if (chattext == "an" && chatname == controller)
    annoying = !annoying;
  if (chatname == controller && chattext == "test")
    {
      sendpm(controller, trivia.cura);
      cout << "------" << trivia.cura << "------" << endl;
      cout << "------" << trivia.cura.length() << endl;
    }

}

int yahoocmd::ProcessCommand(string mycmd)
{
  unsigned int namelength, textlength, rating;
  unsigned int place, table;
  unsigned int i;
  string chatname, chattext;
  time_t rawtime;
  char *timestr; //11-18
  time (&rawtime);
  timestr = asctime(localtime(&rawtime));
  for (i = 11; i <= 18; i++)
    timestr[i-11] = timestr[i];
  timestr[8] = '\0';
  
  unsigned int commandlength = 256 * (uint32_t)(mycmd[5]) + (uint32_t)(mycmd[6]);
  switch(mycmd[7])
    {
    case 'c': // someone talked in lobby
     // 100 0 ID1 ID2 ID3 PL1 PL2 c NL1 NL2 name TL1 TL2 text
     //   0 1  2   3   4   5   6  7  8  9   10   
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      textlength = (uint32_t)(mycmd[11 + namelength]) + 
	256 * (uint32_t)(mycmd[10 + namelength]);
      chattext = mycmd.substr(12 + namelength, textlength);
      
      cout << "(" << timestr << ") " << chatname << ": " << chattext << endl;
     
      if (chattext[0] == '!') process_pm(chatname, chattext.substr(1, chattext.length()-1));
      if (echomode == 1) 
	if (chatname != username) 
	  //    obfuscate();
	  chatsay(chattext);
      if (triviamode == 1 && cmp_answers(chattext, trivia.cura)) trivia.got_answer(chatname, removespaces(chattext));
      
      if (chattext == "/trivia" && triviamode == 0 && gamemode.trivia == 1)
	{
	  triviamode = 1;
	  trivia.start();
	}

	//trivia.trivia_handler(chatname, chattext);

      break;
      
    case '3': // someone sent a PM
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      textlength = (uint32_t)(mycmd[11 + namelength]) + 
	256 * (uint32_t)(mycmd[10 + namelength]);
      chattext = mycmd.substr(12 + namelength, textlength);
  
      process_pm(chatname, chattext);

      break;
    case 'x': // someone left the room
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);

      if (chatname == username)

	{
	  cout << "   SERVER DISCONNECTED" << endl;
	  //attempt reconnect
	}
      if (!broadcast) break;

      
      playerclass.removeplayer(chatname);
      cout << chatname << " left." << endl;

      break;
    case 'e': // someone entered the room
      if (chatsayenabled && !broadcast) break;
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      if (chatname == username) 
	{
	  chatsayenabled = 1;
	  cout << "I ENTERED" << endl;
	  if (gamemode.trivia) chatsay("To start trivia please do /trivia");
	}

      if (!broadcast) break;
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      playerclass.addplayer(chatname, 0);
      cout << chatname << " Entered." << endl;
      if (mathmode)
	chatsay ("goodday, " + chatname + "s");
      break;
    case 't': // someone's rating changed
      if (!broadcast) break;
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      rating = (uint32_t)(mycmd[10+namelength])*256 + 
	(uint32_t)(mycmd[11+namelength]);
      if (rating == 32768) 
	rating = 0;
      playerclass.modifyplayer(chatname, rating);
      break;
    case 'j': // someone entered a table
      if (!broadcast) break;
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      table = (uint32_t)(mycmd[10 + namelength]);
      cout << chatname << " entered table " << table << endl;
      break;
    case 'l': // someone left a table
      if (!broadcast) break;
      namelength = (uint32_t)(mycmd[9]);
      chatname = mycmd.substr(10, namelength);
      table = (uint32_t)(mycmd[10 + namelength]);
      cout << chatname << " left table " << table << endl;
      break;
    case 's': // someone took a seat at a table or a seat is available
      if (!broadcast) break;
      table = (uint32_t)(mycmd[8]);
      place = (uint32_t)(mycmd[9]) + 1;
      if ((uint32_t)(mycmd[6]) == 5)
	cout << "Table " << table << " now has seat " << place << " available.\n";
      else
	{
	  namelength = (uint32_t)(mycmd[11]);
	  chatname = mycmd.substr(12, namelength);
	  cout << chatname << " took seat " << place << " at table " << table << "\n";
	}
      break;
    case 'd': // table does nto exist anymroe
      if (!broadcast) break;
      table = (uint32_t)(mycmd[8]);
      cout << "Table " << table << " exists no more." << endl;
      //tableclass.removetable(table);
      break;
    case 'p': // table privacy settings
      if (!broadcast) break;
      table = (uint32_t)(mycmd[8]);
      place = (uint32_t)(mycmd[9]);
      if (place == 0) cout << "Table " << table << " is public." << endl;
      if (place == 1) cout << "Table " << table << " is protected." << endl; 
      if (place == 2) cout << "Table " << table << " is private." << endl;
    
      break;
    case '0': // change in table parameters (should be only timer)
      if (!broadcast) break;
      table = (uint32_t)(mycmd[8]);
      get_table_parameters(mycmd.substr(7, commandlength));
      break;
    case 'n': // new table
      if (!broadcast) break;
      table = (uint32_t)(mycmd[8]);
      get_table_parameters(mycmd.substr(7, commandlength));
      break;
    case '=': // command at a table
      processtablecommand(mycmd, commandlength);
      break;
    default:
      //  printbytes(mycmd.substr(7, commandlength));
      ; 
    }
 // els
  //printbytes(mycmd.substr(7, 256 * (int)mycmd[5] + (int)mycmd[6]));
  return 1;
}

void yahoocmd::processtablecommand(string mycmd, int commandlength)
{
  int table, namelength, textlength, place, icon, count255, num_passed_tiles;
  int curtile = 0;
  string chatname, chattext;
  unsigned int i;
  char mylet;
  table = (uint32_t)(mycmd[8]);
  vector <char> passed_tiles;
  switch(mycmd[9])
    {
    case 'c':
      
      namelength = (uint32_t)(mycmd[11]);
      chatname = mycmd.substr(12, namelength);
      textlength = (uint32_t)(mycmd[13 + namelength]) + 
	256 * (uint32_t)(mycmd[12 + namelength]);
      chattext = mycmd.substr(14 + namelength, textlength);
      cout << "[table " << table << "] " 
	   << chatname << ": " << chattext << endl;	  
      if (chattext == "help" &&
	  (chatname == curplayer || chatname == controller))
	help_player(table, 0);
      if (chattext == "tiles" &&
	  (chatname == curplayer || chatname == controller))
	help_player(table, 1);
      if (chattext.substr(0, 6) == "check ")
	{
	  chattext = chattext.substr(6, chattext.length()-6); 
	  if (mylit.mytrie.findword(chattext)==0)
	    chatsaytable(table, chattext + " is NOT a word in my dictionary.");
	  else
	    chatsaytable(table, chattext + " is a word in my dictionary.");
	}
      if (chattext.substr(0, 7) == "define ")
	{
	  chattext = chattext.substr(7, chattext.length()-7);
	  string mydef = define(chattext);
	  mydef = extract_def(mydef);
	  chatsaytable(table, mydef);
	}
      if (chatname == controller) process_pm(chatname, chattext.substr(1, chattext.length()-1));

      break;
    case 'p':
      // placed a letter for the first time!
      //cout << "TABLE " << table << " --- placed " << 
      //char((uint32_t)(mycmd[11]) + 'a') << " on " << (uint32_t)(mycmd[12]) << ", " 
      //   << (uint32_t)(mycmd[13]) << endl;
      if (numplacedtiles == 0)
	// start a new representation
	clear_temprep();
      
      temprep[(uint32_t)(mycmd[12])][(uint32_t)(mycmd[13])] = 
	char((uint32_t)(mycmd[11]) + 'A');
      numplacedtiles++;
      //printbytes(mycmd.substr(7, commandlength));
      break;
    case 'u':
      // cout << "TABLE " << table << " --- space " << (uint32_t)(mycmd[11]) << ", "
      //   << (uint32_t)(mycmd[12]) << endl;
      numplacedtiles--;
      temprep[(uint32_t)(mycmd[11])][(uint32_t)(mycmd[12])] = -1; // clear
      //printbytes(mycmd.substr(7, commandlength));
      break;
    case 'm':
      //cout << "TABLE " << table << " --- the letter previously on " << 
      //(uint32_t)(mycmd[11]) << ", " << (uint32_t)(mycmd[12]) << " is now on " <<
      //(uint32_t)(mycmd[13]) << ", " << (uint32_t)(mycmd[14]) << endl;
      mylet = temprep[(uint32_t)(mycmd[11])][(uint32_t)(mycmd[12])];
      temprep[(uint32_t)(mycmd[11])][(uint32_t)(mycmd[12])] = -1;
      temprep[(uint32_t)(mycmd[13])][(uint32_t)(mycmd[14])] = mylet;
      break;
    case 'h':
      //cout << "TABLE " << table << " --- the blank on " << (uint32_t)(mycmd[11]) 
      //   << ", " << (uint32_t)(mycmd[12]) << " became '" 
      //   << char((uint32_t)(mycmd[13]) - 26 + 'a') << "'" << endl;
      temprep[(uint32_t)(mycmd[11])][(uint32_t)(mycmd[12])] = 
	char ((uint32_t)(mycmd[13])-26+'a');
      break;
    case '5':
      namelength = (uint32_t)(mycmd[11]);
      chatname = mycmd.substr(12, namelength);
      cout << "TABLE " << table << " --- the host of this table is " << chatname
	   << endl;
      break;
    case 'n':
      textlength = (uint32_t)(mycmd[11]);
      chattext = mycmd.substr(12, textlength);
      cout << "TABLE " << table << " --- submitted the bad word '" << chattext
	   << "'\n";
      numplacedtiles = 0;
      // DO NOT SWITCH TURN
      stillmyturn = 1;
      break;
    case 't':
      place = (uint32_t)(mycmd[10]);
      icon = (uint32_t)(mycmd[11]);
      namelength = (uint32_t)(mycmd[13]);
      chatname = mycmd.substr(14, namelength);
      cout << "TABLE " << table << " --- " << chatname << " is sitting in seat "
	   << place + 1 << " with icon " << icon << endl;
      if (litmode == 1)
	{
	  if (chatname == username) myseat = place;
	  if (chatname != username && gamemode.rated == 0)
	    {
	        chatsaytable(table, "Hi, " 
			   + chatname + 
			   ", I am the fastest program in Literati! Press start to play me. Don't worry, this game is UNRATED.");
	      curplayer = chatname;
	    }
	}
      break;
    case 'o':
      place = (uint32_t)(mycmd[10]);
      namelength = (uint32_t)(mycmd[12]);
      chatname = mycmd.substr(13, namelength);
      cout << "TABLE " << table << " --- " << chatname << " is ";
      if (place == 1) cout << "kibitzing" << endl;
      if (place == 0) cout << "not kibitzing" << endl;
      break;
    case '[':
      place = (uint32_t)(mycmd[10]);
      cout << "TABLE " << table << " --- " << place << " resigned!" << endl;
      //if (gamemode.continuous == 1) pressstart(table);
      break;
      
    case 'b':
      wordtoplay = 0;
      cout << "TABLE " << table << " --- game started!" << endl;
      clear_temprep();
      mylit.clear_gameboard();
      get_tiles(mycmd, 0);
      
      if (litmode == 1)
	{
	  // i start the game!!!!
	  if (gamemode.rated == 0)
	    {
	      chatsaytable(table, "Good luck, " + curplayer + "!");
	      chatsaytable(table, "If at any point you want help, type help to see what your best play is, or type tiles to see the remaining tiles.");
	    }
	  if (curturn == myseat) makeplays(table);
	}//this is red
      
      
      break;
      
    case 'a':
      //opening table string
      numplacedtiles = 0;
      stillmyturn = 0;
      cout << "TABLE " << table << " ---- OPENING TABLE STRING ----- " << endl;
      cout << "LENGTH " << (uint32_t)(mycmd[5]) * 256 + (uint32_t)(mycmd[6]) << endl;
      //	  cout << "[";
      count255 = 0;
      for (i = 0; i < (uint32_t)(mycmd[5]) * 256 + (uint32_t)(mycmd[6]) + 7; i++)
	{
	  place = (uint32_t)(mycmd[i]);
	  if (place!= 255) 
	    {
	      cout << place;
	      //  if (i == 6) cout << "] ";
	      //else 
	      cout << " ";
	    }
	  if (place == 255)
	    count255++;
	  if (count255 > 0 && place != 255)
	    {
	      cout << "<" <<i << ", " << count255 << "> ";
	      count255 = 0;
	    }
	}
      cout << endl 
	   << " --------------------------------------------------- " << endl;
      
      get_tiles(mycmd, 1);
      
      break;
    case 's':
      // pass tiles
      clear_temprep();
      num_passed_tiles = (uint32_t)(mycmd[14]);
      
      
      if ( num_passed_tiles > 0)
	{
	  // figure out randomization
	  while (curtile < num_passed_tiles)
	    {
	      passed_tiles.push_back(mycmd[18 + (curtile*4)]);
	      curtile++;
	    }
	  // passed_tiles holds tiles that were passed.
	  pass_tiles(passed_tiles, table);
	  
	  
	}
      
      
      
      if (curturn == 1) curturn = 0;
      else curturn = 1;
      wordtoplay = 0;
      if (curturn == myseat && litmode == 1)
	makeplays(table);
      break;
    case 'f':
      // curturn submitted
      if (stillmyturn == 1)
	{
	  stillmyturn = 0;
	  cout << curturn << " submitted bad word so still his turn\n";
	  clear_temprep();
	  wordtoplay++;
	  if (curturn == myseat) playtiles(table);
	  break;
	}
      wordtoplay = 0;
      cout << "Player " << curturn << " submitted." << endl;
      
      // evaluate word, get new tiles, then set numplacedtiles = 0
      for (int i = 0; i < 15; i++)
	{
	  for (int j = 0; j < 15; j++)
	    {
	      if (temprep[i][j] != -1)
		{
		  mylit.gameboard[j][i] = temprep[i][j];
		  int j1 = find_in_player_tileset(temprep[i][j]);
		  playertiles[curturn].erase(playertiles[curturn].begin()+j1, 
					     playertiles[curturn].begin()+j1 + 1);
		  
		}
	    }
	}
      get_new_tiles(table);
      if (curturn == 1) curturn = 0;
      else curturn = 1;	  
      
      // check now if it's MY TURN if lit bot is on
      // if it is, call makeplays
      if (curturn == myseat && litmode == 1)
	makeplays(table);
      numplacedtiles = 0;
      break;
      
    default:
      printbytes(mycmd.substr(7, commandlength));
      ;
    }
}

void yahoocmd::get_table_parameters(string mycmd)
{
  // ch  = challenge; NP = number players; RD = rated; PL = 1 if no 3 min timer
  // IT and TT signify timer
  int ch, np, rd, pl;
  // printbytes(mycmd);
  string it, tt;
  char parambuffer[1000];
  char parambuffer2[1000];
  int untimed = 0;
  unsigned int itloc, ttloc;
  sprintf(parambuffer, "Table %d's parameters: ", (uint32_t)(mycmd[1]));
  if (mycmd.find("ch", 0) != string::npos) ch = 1;
  else ch = 0;
  if (mycmd.find("rd", 0) != string::npos) rd = 1;
  else rd = 0;
  if (mycmd.find("pl", 0) != string::npos) pl = 1;
  else pl = 0;
  np = mycmd.find("np", 0);
  np = mycmd[np + 4] - '0';
  
  if ( (itloc = mycmd.find("it", 0)) != string::npos)
    {
      assert( (ttloc = mycmd.find("tt", 0)) != string::npos); 
      // cant have one without other
      it = mycmd.substr(itloc + 4, mycmd[itloc + 3]);
      tt = mycmd.substr(ttloc + 4, mycmd[ttloc + 3]);
    }
  else
    untimed = 1;
  
  if (ch) strcat(parambuffer, "Challenge mode on. ");
    
      
  if (rd) strcat(parambuffer, "Rated. ");  
  else strcat(parambuffer, "Unrated. ");
  if (pl) strcat(parambuffer, "No 3-min timer. ");
  if (untimed) strcat(parambuffer, "Untimed. ");
  else
    {
      if (atoi(tt.c_str()) == -1) strcat(parambuffer, "Untimed. ");
      else
	{
	  sprintf(parambuffer2, "Timer: %d/%d ", atoi(tt.c_str()) /60000, 
		  atoi(it.c_str()) / 1000);
	  strcat(parambuffer, parambuffer2);
	}
    }
  sprintf(parambuffer2, "%d players.", np);
  strcat(parambuffer, parambuffer2);
  cout << parambuffer << endl;
  if (annoying) chatsay(parambuffer);


}

int yahoocmd::ExtractCommands(string mycmd)
{
  int stillreading = 1;
  unsigned int length;

  static string lastcommand; //initialized to 0
  mycmd = lastcommand + mycmd;
  
  while (stillreading == 1)
    {
      // all commands are of the following form
      // 100 0 ID1 ID2 ID3 L1 L2 rest of packet
      // L1 L2 signify the length of the rest of packet
      if (mycmd.size() >= 7)   
	{
	  length = (uint32_t)(mycmd[5]) * 256  + (uint32_t)(mycmd[6]);

	  if (mycmd.size() < length + 7) // if the command is not complete (broken)
	    {
	      lastcommand = mycmd;      // save command so we can add to it next call
	      stillreading = 0;         // exit loop
       	    }
	  else               // else the comand is complete
	    {
	      ProcessCommand(mycmd); // process it
	     	      
	      mycmd = mycmd.substr(length + 7, mycmd.size() - length - 7);
	      // and then take next command out of the large mycmd string
	      if (mycmd.size() == 0) // if this next command is unexistent
		{
		  lastcommand.clear(); // clear the save list and leave
		  stillreading = 0;
		}
	    }     
	} 
      else // the command is not complete so save and add onto it next time
	{
	  lastcommand = mycmd;
	  stillreading = 0;
	}      
    }
  //  cout << "OUT OF LOOP " << endl;
  return 1;
}

uint32_t yahoocmd::get_rand(int i, int index)
{
  if (index == 0) N++;
  return myrand[index].Ip(i);
}

uint64_t yahoocmd::get_seed(string mycmd)
{
  // 10 - 17
  return ( 
	  (uint64_t)(mycmd[17] & 0xFF) |
	  (uint64_t)(mycmd[16] & 0xFF) << 8 |
	  (uint64_t)(mycmd[15] & 0xFF) << 16 |
	  (uint64_t)(mycmd[14] & 0xFF) << 24 |
	  (uint64_t)(mycmd[13] & 0xFF) << 32 |
	  (uint64_t)(mycmd[12] & 0xFF) << 40 |
	  (uint64_t)(mycmd[11] & 0xFF) << 48 |
	  (uint64_t)(mycmd[10] & 0xFF) << 56);
}

void yahoocmd::get_tiles(string mycmd, int firsttime)
{
  mylit.tiles.clear();
  playertiles[0].clear();
  playertiles[1].clear();
  uint32_t a;
  int blah;  
  int myindex = 0;
  if (firsttime)
    {
      uint64_t seed = get_seed(mycmd);
      uint32_t numtilesplayed = 0;
      cout << "Seed: " << seed << endl;
      
      myindex = 1;  // future
      myrand[0].seed(seed);
      myrand[1].seed(seed);
      numtilesplayed = (uint32_t)(mycmd[21])+ (uint32_t)(mycmd[20])*256+ (uint32_t)(mycmd[19])*
	256*256 + (uint32_t)(mycmd[18])*256*256*256;

      cout << "Num tiles played: " << numtilesplayed << endl;
      for (a = 0; a < numtilesplayed; a++)
	{
	  get_rand(2, 0);
	  get_rand(2, 1);
	}

    }
  startplaying = 0; // begin      

      /*
      cout << "Random seed: " << seed << "\t" 
	   << "Numtilesplayed " << numtilesplayed <<"\n";
      cout << a1 << "\t" << a2 << "\t" << a3 << "\t" << a4 << "\t" << (uint32_t)(mycmd[12])
      << "\t" << (uint32_t)(mycmd[13]) << endl;*/
      unsigned int t;

      int j[26] =
	{ 71820, 18890, 35684, 36016, 110868, 13271, 28960, 21212, 80345, 1910, 10221,
	  48908, 25845, 62648, 57377, 27023, 1691, 67392, 90161, 59171, 32528, 9494,
	  8935, 2855, 14188, 4058};
      // adds up to 941471
      for (int i = 0; i < 109; i++)
	{
	  int i1 = get_rand(941471, myindex);
	  int k1 = 0;
	  int j2 = -1;
	  do
	    {
	      j2++;
	      k1+= j[j2];
	    } while (k1 < i1);
	  mylit.tiles.push_back( (char)j2+'A');
	}
      // now insert blanks                                                                 
      vector <char>::iterator myit = mylit.tiles.begin();
      for (int j1 = 0; j1 < 2; j1++)
	{
	  a = get_rand(mylit.tiles.size(), myindex);
	  myit +=a;
	  mylit.tiles.insert(myit, 1, '?');
	  myit = mylit.tiles.begin();
	}
      /*
	myit = tiles.end();
	tiles.insert(myit, 1, '?');
	myit -= 6;
	tiles.insert(myit, 1, '?');*/
        for (int x = mylit.tiles.size() - 1; x >= 0; x--)
	{
	  cout << mylit.tiles[x];
	}
      
      cout << endl;
      
      playertiles[0].clear();
      playertiles[0].push_back('?');
      playertiles[1].clear();
      playertiles[1].push_back('?');
      a = get_rand(2, myindex);
      cout << "Not the turn of: " << a << endl;

      if (a == 1) curturn = 0;
      if (a == 0) curturn = 1;
      for (blah = 0; blah <= 1; blah++)
	for (t = 1; t < 7; t++)
	  {
	    playertiles[blah].push_back(mylit.tiles[mylit.tiles.size()-1]);
	    mylit.tiles.pop_back();
	  }
	    
      
      for (blah = 0; blah <= 1; blah++)
	{
	  cout << "Tiles for player on seat " << blah << endl;       
	  for (t = 0; t < playertiles[blah].size(); t++)
	    {
	      cout << playertiles[blah][t];
	    }
	  cout << endl;
	}
}

int yahoocmd::ProcessReply(string reply, int len)
{
  string strtosend;
  string gameroom = GAMEROOM;
  string commandtosend, encryptedcookie;
  string mycmd;
  vector <string> cmds;

  //  while(reply.length()!=len){};
  //  cout << reply << " " <<  reply.length() << endl;

  if (state == CONNECTED)
    {
      reply = encryptdata(reply, RECEIVE_KEY);
      ExtractCommands(reply);
      return 1;
    }
  else
    switch(state)
      {
      case FIRSTMSG:
	if (reply == "YAHOO!")
	  {
	    client.send("Y", 1); // send it a Y to get key.
	    state = SENTY;
	    return 1;
	  }
	break;
      case SENTY:
	if (reply.length() == 8)
	  {
	    // this is the key
	    // generate send, receive keys.
	    keygen(reply );
	    state = GOTKEY;
	    
	    strtosend = "o" + writeutf(gameroom);
	    strtosend =  encryptdata(strtosend, SEND_KEY);
	    client.send( strtosend.c_str(), strtosend.size());
	    
	    state = SENTJOINREQUEST;
	    return 1;
	  }
	break;
      case SENTJOINREQUEST:
        reply = encryptdata(reply, RECEIVE_KEY);
	
	if (reply.substr(3, reply.length() - 7) == gameroom)
	  {
	     myid = reply.substr(reply.length() - 3, 3);
	     packetheader = "d";
	     packetheader += CHR(0);
	     packetheader += myid;
	     state = RECEIVED_ID;
	     cout << "  ID:  " << 
	       (uint32_t)(myid[0]) << " " << 
	       (uint32_t)(myid[1]) << " " << 
	       (uint32_t)(myid[2]) << endl;
	     return 1;
	  }
	cout << "No id?" << endl;
	return 0;
	break;
      case RECEIVED_ID:
	reply = encryptdata(reply, RECEIVE_KEY);
	if (reply.find("GAMES") != string::npos)
	  {
	    // here we get cookie and send it.
	    
	    string functionstring(1, CHR(0));
	    string addinfo = "Mozilla/4.0(compatible; MSIE 6.0; Windows NT 5.1)"; 
	    commandtosend = packetheader + writeutf(functionstring + writeutf("id=" + username) + 
						    writeutf(mycookie) + writeutf(addinfo) + writeutf("us"));

	    
	    encryptedcookie = encryptdata(commandtosend, SEND_KEY);
	    
	    
	    client.send (encryptedcookie.c_str(), encryptedcookie.length());
	    state = CONNECTED;
	    cout << "Sent encrypted cookie. " << endl;
	  }
	return 1;
	break;
      default:
	cerr << "SHOULDN'T BE HERE" << endl;
	  
      }
  return 1;
}

string yahoocmd::extractcookie (string mycookiestring)
{
  int myloc, mylocsemicolon;
  string part1, part2;
  //  cout << mycookiestring.length() << endl;
  //cout << mycookiestring << endl;
  if (mycookiestring.find("Y=v=", 0) == string::npos)
    {
      cout << "Cookie invalid" << endl;
      return "";
    }
  cout << "Cookie is valid. " << endl;
  
  myloc = mycookiestring.find("Y=v", 0);
  mycookiestring = mycookiestring.substr(mycookiestring.length() - 
   (mycookiestring.length() - myloc + 1), mycookiestring.length() - myloc + 1);
  mylocsemicolon = mycookiestring.find(";", 0);
  part1 = mycookiestring.substr(1, mylocsemicolon);
  
  myloc = mycookiestring.find("T=z", 0);
  mycookiestring = mycookiestring.substr(mycookiestring.length() - 
   (mycookiestring.length() - myloc + 1), mycookiestring.length() - myloc + 1);

  mylocsemicolon = mycookiestring.find(";", 0);
  part2 = mycookiestring.substr(1, mylocsemicolon - 1);

  mycookiestring = part1;
  mycookiestring += " ";
  mycookiestring += part2;

  //  cout << mycookiestring << endl;
  return mycookiestring;
}

int yahoocmd::getcook()
{

 
  string requeststring;

  int bytesReceived;
  char cookiereply[RCVBUFSIZE + 1];
  string cook;
  try
     {
       TCPSocket cookie_socket (LOGINSERVER, 80);
       requeststring = "GET /config/login?.src=&login="; 
       requeststring+= username ; requeststring += "&passwd=";
       requeststring += password; requeststring +=  "&n=1 HTTP/1.0\r\n";
       requeststring += "Accept: */*\r\n";
       requeststring += "Accept: text/html\r\n";
       requeststring += "\r\n";
       cookie_socket.send( requeststring.c_str(), (int)requeststring.length());
       //    cout << "Sent " << requeststring << endl;
     
	 // Receive up to the buffer size bytes from the sender
       if ((bytesReceived = (cookie_socket.recv(cookiereply, RCVBUFSIZE))) <= 0) {
	 cerr << "Unable to read";
	 exit(1);


	 


       }
       cook = cookiereply;
       mycookie = extractcookie(cook);
       if (mycookie == "")
	 {
	   return 0;
	 }
       //      cookie_socket.close();
     }
  catch ( SocketException& e)
    {
      cout << "Exception was caught! " << e.what() << "\n";
    }
  cout << "Cookie: " << mycookie << endl;
  return 1;
}
string yahoocmd::define(string myword)
{
  string requeststring;

  int bytesReceived;
  char dicreply[RCVBUFSIZE + 1];
  try
     {
       TCPSocket dic_socket ("64.39.29.217", 80);
       requeststring = "GET /word/" + myword;
       requeststring += " HTTP/1.1\r\n";
       requeststring += "Host: www.morewords.com\r\n";
       requeststring += "Connection: keep-alive\r\n";
       requeststring += "Accept: */*\r\n";
       requeststring += "Accept: text/html\r\n";
       requeststring += "\r\n";
       dic_socket.send( requeststring.c_str(), (int)requeststring.length());
       cout << "Sent " << requeststring << endl;
     
	 // Receive up to the buffer size bytes from the sender
       if ((bytesReceived = (dic_socket.recv(dicreply, RCVBUFSIZE))) <= 0) {
	 cerr << "Unable to read";
	 exit(1);
       }
       //cout << dicreply << endl;
     }
  catch ( SocketException& e)
    {
      cout << "Exception was caught! " << e.what() << "\n";
    }
  return dicreply;


}

string yahoocmd::extract_def(string mystr)
{
  if (mystr.find("No words found in this wordlist", 0)!=string::npos)
    return "That word was not found in this wordlist.";

  if (mystr.find("is a valid word in this word list", 0)!=string::npos)
    return "That word is valid, but no definition has been loaded for it yet.";
  // find first occurrence of italics
  int i = mystr.find("</i>", 0);
  int j = mystr.find("<small>", 0);
  return mystr.substr(i+4, j-(i+4));

}

