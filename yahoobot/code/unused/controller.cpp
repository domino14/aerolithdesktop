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

struct termios save;

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
  cout << "controller [-options]" << endl;
  cout << "      Default name = yourm0mz" << endl;
  cout << "      Default gameroom = games.room.literati_int_c" << endl;
  cout << "      Default server = 66.218.68.247" << endl;
  cout << "      Options: " << endl;
  cout << " -u username   sign on with username" << endl;
  cout << " -g gameroom   change gameroom" << endl;
  cout << " -s server     sign on to server" << endl;
}

int main ( int argc, char *argv[] )
{
  string username = "yourm0mz", 
    password, gameroom = "games.room.literati_int_c", 
    gameserver = "66.218.68.247", controller = "";
  Gamemode gamemode = {'\0','\0','\0', '\0', '\0'};

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
      while ((optind < argc) && (argv[optind][0]=='-'))
	{
	  string sw = argv[optind];
	  if (sw == "-u")
	    {
	      optind++;
	      username = argv[optind];
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
      cout << endl;
      cout << "Connecting to " << gameroom << " with id " << username
	   << " as controller.";
      cout << endl;
    

      yahoocmd myclient;
      myclient.init(username, password, controller, gameroom, gameserver, gamemode);
      myclient.start_connection(0);
      string mystr;
      string slave;
      cout << "Enter username to control: ";
      cin >> slave;
      while(1)
	{	  
	  cin >> mystr;
	  myclient.sendpm(slave, mystr);
	}

    }

  return 0;
}
