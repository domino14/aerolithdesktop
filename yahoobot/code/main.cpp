// programmed by C3sar

using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <termios.h>
#include <unistd.h>

#include "yahoo.h"
#include "trivia.h"

struct termios save;
yahoocmd myclient; // make GLOBAL
Trivia trivia;

void echoOff()
{
  struct termios term;
  tcgetattr( 0, &save );
  term = save;
  term.c_lflag &= (~ECHO);
  tcsetattr( 0, TCSANOW, &term );
}

void echoOn()
{
  tcsetattr( 0, TCSANOW, &save );
}

void usage()
{
  cout << "Usage: " << endl;
  cout << "ybot [-options]" << endl;
  cout << "      Default name = cesarsa1ad" << endl;
  cout << "      Default controller = yourm0mz" << endl;
  cout << "      Default gameroom = games.room.literati_int_c" << endl;
  cout << "      Default server = 209.73.170.129" << endl;
  cout << "      Options: " << endl;
  cout << " -u username   sign on with username" << endl;
  cout << " -c controller use controller " << endl;
  cout << " -g gameroom   change gameroom" << endl;
  cout << " -s server     sign on to server" << endl;
  cout << " -lowball      lowball mode on" << endl;
  cout << " -broadcast    some debug info" << endl;
  cout << " -rated        using for a rated game, press any key after the first move to play" << endl;
  cout << " -trivia       say start trivia at beginning" << endl;
}

int main ( int argc, char *argv[] )
{
  string username = "cesarsa1ad", 
    password, gameroom = "games.room.literati_int_c", 
    gameserver = "209.73.170.129", controller = "yourm0mz";
  Gamemode gamemode = {'\0','\0','\0', '\0', '\0', '\0', '\0'};

  //      if ( strcmp(argv[1], "debug") == 0)
  //{
  //  debugmode = 1;
  //}
  // else
  //{
  

  if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
      usage();
      return 0;
    }
  
  else
    {
      int optind=1;

      myclient.setbroadcast(0);
      while ((optind < argc) && (argv[optind][0]=='-'))
	{
	  string sw = argv[optind];
	  if (sw == "-u")
	    {
	      optind++;
	      username = argv[optind];
	    }
	  else if (sw == "-c")
	    {
	      optind++;
	      controller = argv[optind];
	    }
	  else if (sw == "-g")
	    {
	      optind++;
	      gameroom = argv[optind];
	    }
	  else if (sw == "-s")
	    {
	      optind++;
	      gameserver = argv[optind];
	    }
	  else if (sw == "-lowball") gamemode.lowball = (char) 1;
	  else if (sw == "-cont") gamemode.continuous = (char) 1;
	  else if (sw == "-lit") gamemode.lit = (char) 1;
	  else if (sw == "-rated") gamemode.rated = (char) 1;

	  else if (sw == "-broadcast") myclient.setbroadcast(1);
	  else if (sw == "-trivia") gamemode.trivia = (char) 1;
	  else
	    {
	      cout << "Unknown switch: " << argv[optind] << endl;
	      return 0;
	    }
	  optind++;
	}
	  
      cout << "Please enter a password for username " << username << ": " << flush;

      echoOff();
      cin >> password;
      echoOn();      

    
      myclient.init(username, password, controller, gameroom, gameserver, gamemode);
      trivia.init();
      myclient.start_connection(1);
    }

  return 0;
}
