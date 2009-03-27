#include "trivia.h"
#include "yahoo.h"
#include <algorithm>
#include <math.h>


extern yahoocmd myclient;

int Trivia::numqs;
vector <string> Trivia::questions;
vector <string> Trivia::answers;
trivia_state Trivia::curstate;
string Trivia::curq, Trivia::cura, Trivia::curh;
timeval Trivia::rawtime;

vector <score> Trivia::scorelist;
int Trivia::round_numqs;
int Trivia::round_curq;

vector <timescore> hof_fastest; // 10 fastest times
vector <score> hof_highest; // 10 highest scores per play
vector <score> hof_best; // 10 best scores per GAME


void load_high_scores();
int manage_high_scores(string chatname, double score, double timeelapsed, char gameoverflag);


bool operator<(const score& x, const score& y)
{
  return x.gen_score > y.gen_score;
}

bool operator<(const timescore& x, const timescore& y)
{
  return x.time < y.time;
}


int Trivia::init()
{
  get_qs();
  load_high_scores();
  return 0;
}      

int Trivia::get_qs()
{
  ifstream i_f;
  char buf[10000];
  string myline;
  string question, answer;
  int i;
  numqs = 0;
  i_f.open("trivia.txt");
  while (!i_f.eof())
    {
      i_f.getline(buf, 10000);
      myline = buf;
      i = myline.find("*", 0);
      question = myline.substr(0, i);
      answer = myline.substr(i+1, myline.size() - i-1);
      questions.push_back(question);
      answers.push_back(answer);
      numqs++;
    }
  i_f.close();
  cerr << numqs << " questions." << endl;
  srand(time(NULL));
  
  return numqs;
}

int Trivia::NewQuestion()
{
  
  // just access thru q number and curh
  int qnum;
  unsigned int i;
  qnum = rand() % numqs;
  curq = questions[qnum];
  cura = answers[qnum];
  curh = cura.substr(0, cura.size() / 2);
  
  for (i = 0; i < curh.size(); i++)
    curh[i] = toupper(curh[i]);
  
  for (i = 0; i < curq.size(); i++)
    curq[i] = toupper(curq[i]);
  
  



  round_curq++;


  return qnum;
}

int Trivia::start()
{
  //  mytimer = time(NULL);
  curstate = STATE_STARTING;
  
  //  msgsent = 0;
  signal (SIGALRM, event_handler);
  alarm(STATE_STARTING_TIMER); // trivia bot starting in 20 seconds. the caller should handle this message.
  char buf[1000];
  sprintf(buf, "*****      TRIVIA BOT STARTING IN %d SECONDS", STATE_STARTING_TIMER);
  string tempstr(buf);
  myclient.chatsay(tempstr);
  myclient.chatsay("Cesar's Trivia Bot Version 3.0");
  myclient.chatsay("Running on Linux, created in c++ with Emacs (vi sucks)");
  if (rand()% 5 == 1) {
    myclient.chatsay("Current records: ");
    sprintf(buf, "Fastest answer: %s with %f seconds...", hof_fastest[0].name.c_str(), hof_fastest[0].time);
    myclient.chatsay(string(buf));
    sprintf(buf, "Highest score on a single question: %s with %f points...", hof_highest[0].name.c_str(), hof_highest[0].gen_score);
    myclient.chatsay(string(buf));
    sprintf(buf, "Highest overall score in a game: %s with %f points...", hof_best[0].name.c_str(), hof_best[0].gen_score);
    myclient.chatsay(string(buf));
    //    myclient.chatsay("To see all the hall of fame records, visit http://t.ath.cx");
  }


  scorelist.clear();
  round_numqs = 15;
  round_curq = 1;
  return 1;
}

int Trivia::get_num_words()
{
  // get num words in cura
  // count spaces
  int spcount = 0;
  for (unsigned int i = 0; i < cura.length(); i++)
    if (cura[i] == ' ') spcount++;

  return spcount + 1;
}
void Trivia::event_handler(int sig)
{
  
  /* this is the timer event handler*/
  signal(SIGALRM, SIG_IGN);  
  int interval;
  char buf[1000];  
  int numwords;
  switch (curstate)
    {
    case STATE_OFF:
      cerr << "I am off. " << endl;
      break;
      
    case STATE_STARTING:
      
      // game just started, i.e. 30 second starting timer just ran out
    case STATE_WAIT_NEW_Q:
      NewQuestion();

      //    sprintf(buf, "*****    %s    (%d words)", curq.c_str(), 
      //      myclient.chatsay("*****     " + curq);
      numwords = get_num_words();
      sprintf(buf, "*****    %s    (%d word%s)", curq.c_str(), numwords, (numwords > 1 ? "s" : "")); 
      myclient.chatsay(string(buf));
      gettimeofday(&rawtime, NULL);
      interval = STATE_IN_Q_NO_HINT_TIMER;
      curstate = STATE_IN_Q_NO_HINT;
      break;
      
    case STATE_IN_Q_NO_HINT:
      // hint timer ran out, should say hint
      myclient.chatsay("*****     HINT: " + curh + "...");
      interval = STATE_IN_Q_AFTER_HINT_TIMER;
      curstate = STATE_IN_Q_AFTER_HINT;
      break;
      
    case STATE_IN_Q_AFTER_HINT:
      myclient.chatsay("*****    Time ran out! The answer was " + cura);
           
      if (round_curq > round_numqs)
	{
	  myclient.chatsay("*****   That was the final question of the round. Scores coming in 8 seconds."); 
	  interval = 8;
	  curstate = STATE_WAITING_FOR_SCORES;
	  break;
	}


      sprintf(buf, "Question %d of %d coming in a few seconds....", round_curq, round_numqs); 
      myclient.chatsay(string(buf));
      interval = STATE_WAIT_NEW_Q_TIMER;
      curstate = STATE_WAIT_NEW_Q;
      break;
    case STATE_WAITING_FOR_SCORES:
      displayscores();
      curstate = STATE_OFF;
      myclient.triviamode = 0;
      alarm(0);
      return;
      break;

    default:
      myclient.chatsay("Unhandled trivia error. Please report to administrator.");
    }
  signal(SIGALRM, event_handler);
  alarm(interval);
  
  
}

int Trivia::displayscores()
{
  sort(scorelist.begin(), scorelist.end());
  myclient.chatsay("********* FINAL SCORES **********");
  char buf[1000];
  
  int numscores = scorelist.size();

  int a = manage_high_scores(scorelist[0].name, scorelist[0].gen_score, 0.0 /*dummy*/, 1);
  if (a == 1)
    {
      sprintf(buf, "*******   %s now has a new winning score record in the Hall Of Fame!!", scorelist[0].name.c_str());
      myclient.chatsay(string(buf));
    }
  for (int i = 1; i < numscores; i++)
    manage_high_scores(scorelist[i].name, scorelist[i].gen_score, 0.0, 1);

  for (int i = 0; i < numscores; i++)
    scorelist.at(i).name = scorelist.at(i).name.substr(0, 6);


  for (int i = 0; i < ceil((float)numscores / 3.0f); i++)
    {
      if (i*3 + 2 < numscores)
      sprintf(buf, "%s - %.2f    %s - %.2f     %s - %.2f", scorelist.at(i*3).name.c_str(), scorelist.at(i*3).gen_score, 
	      scorelist.at(i*3 + 1).name.c_str(), scorelist.at(i*3 + 1).gen_score, scorelist.at(i*3 + 2).name.c_str(), 
	      scorelist.at(i*3 + 2).gen_score);
      else if (i*3 + 1 < numscores)
	sprintf(buf, "%s - %.2f     %s - %.2f", scorelist.at(i*3).name.c_str(), scorelist.at(i*3).gen_score,
		scorelist.at(i*3 + 1).name.c_str(), scorelist.at(i*3 + 1).gen_score);
      else if (i*3 < numscores)
	sprintf(buf, "%s - %.2f", scorelist.at(i*3).name.c_str(), scorelist.at(i*3).gen_score);
      else
	sprintf(buf, "ERROR");


      myclient.chatsay(string(buf));
    }

  myclient.chatsay("Thanks for playing! To start trivia again, type /trivia.");
  // myclient.chatsay("If you want to see the Hall of Fame, please open the following website: http://t.ath.cx"); 
  return 0;
}

void writetohof()
{
  ofstream o_f;
  o_f.open("highscores/hof.dat");
  for (int i = 0; i < 10; i++)
    {
      o_f << hof_fastest[i].name << endl;
      o_f << hof_fastest[i].time << endl;
    }
  for (int i = 0; i < 10; i++)
    {
      o_f << hof_highest[i].name << endl;
      o_f << hof_highest[i].gen_score << endl;
    }
  for (int i = 0; i < 10; i++)
    {
      o_f << hof_best[i].name << endl;
      o_f << hof_best[i].gen_score << endl;
    }

  o_f.close();

  // now write formatted to html file
  o_f.open("highscores/index.html");
  o_f << "<TITLE>Cesar Trivia Hall of Fame</TITLE>";
  o_f << "<TABLE BORDER = 1>" << endl;
  o_f << "<CAPTION> Fastest answer times</CAPTION>";
  for (int i = 0; i < 10; i++)
    {
      o_f << "<TR><TD>" << hof_fastest[i].time << "<TD>" << hof_fastest[i].name << "</TR>" << endl;
    }
  o_f << "</TABLE>" << endl;

  o_f << "<TABLE BORDER = 1>" << endl;
  o_f << "<CAPTION> Highest scores per question</CAPTION>";
  for (int i = 0; i < 10; i++)
    {
      o_f << "<TR><TD>" << hof_highest[i].gen_score << "<TD>" << hof_highest[i].name << "</TR>" << endl;
    }
  o_f << "</TABLE>" << endl;

  o_f << "<TABLE BORDER = 1>" << endl;
  o_f << "<CAPTION> Highest scores per game</CAPTION>";
  for (int i = 0; i < 10; i++)
    {
      o_f << "<TR><TD>" << hof_best[i].gen_score << "<TD>" << hof_best[i].name << "</TR>" << endl;
    }
  o_f << "</TABLE>" << endl;
  o_f.close();


}

void load_high_scores()
{
  ifstream i_f;
  i_f.open("highscores/hof.dat");
  if (!i_f)
    {
      
      // hof.dat doesn't exist, create blank hof
      cout << "hof file doesn't exist, creating one" << endl;
      struct Score tempscore;
      tempscore.name = "machinagod2002";
      
      struct Timescore temptime;
      temptime.time = 60.00;
      temptime.name = "machinagod2002";
      for (int i = 0; i < 10; i++)
	hof_fastest.push_back(temptime);

      tempscore.gen_score = 0.00;
            
      for (int i = 0; i < 10; i++)
	hof_highest.push_back(tempscore);

      tempscore.gen_score = 0.00;
            
      for (int i = 0; i < 10; i++)
	hof_best.push_back(tempscore);
      
      writetohof();
    }

  else
    {
      cout << "reading hof file" << endl;
      struct Score tempscore;
      struct Timescore temptime;
      string blah;
      string blah2;
      for (int i = 0; i < 10; i++)
	{
	  getline(i_f, blah, '\n');
	  getline(i_f, blah2, '\n');
	  temptime.name = blah;
	  temptime.time = atof(blah2.c_str());
	  hof_fastest.push_back(temptime);
	}
      for (int i = 0; i < 10; i++)
	{
	  
	  getline(i_f, blah, '\n');
	  getline(i_f, blah2, '\n');
	  tempscore.name = blah;
	  tempscore.gen_score = atof(blah2.c_str());

	  hof_highest.push_back(tempscore);
	}
      for (int i = 0; i < 10; i++)
	{
	  getline(i_f, blah, '\n');
	  getline(i_f, blah2, '\n');
	  tempscore.name = blah;
	  tempscore.gen_score = atof(blah2.c_str());
	  hof_best.push_back(tempscore);
	}
      writetohof(); // sanity check

    }

}


int manage_high_scores(string chatname, double score, double timeelapsed, char gameoverflag)
{
  int retval = 0;
  if (gameoverflag == 0)
    {
      struct Score tempscore;
      if (score > hof_highest[9].gen_score) // if score is highest than the lowest score in the top 10
	{
	  tempscore.name = chatname;
	  tempscore.gen_score = score;
	  hof_highest.push_back(tempscore);
	  sort(hof_highest.begin(), hof_highest.end()); // this puts highest number at index 0 and lowest number at index 10
	  hof_highest.pop_back(); // pops index 10
	  writetohof();
	  retval = 1;
	}
      struct Timescore temptime;
      if (timeelapsed < hof_fastest[9].time) // if time is lowest than the highest time in the top 10
	{
	  temptime.name = chatname;
	  temptime.time = timeelapsed;
	  hof_fastest.push_back(temptime);
	  sort(hof_fastest.begin(), hof_fastest.end()); // this puts lowest time at index 0 and highest @ index 10
	  hof_fastest.pop_back(); // pops index 10
	  writetohof();
	  retval = 1;
	}
    }
  else
    {
      struct Score tempscore;
      if (score > hof_best[9].gen_score) // if total score is highest than lowest score in top 10
	{
	  tempscore.name = chatname;
	  tempscore.gen_score = score;
	  hof_best.push_back(tempscore);
	  sort(hof_best.begin(), hof_best.end());
	  hof_best.pop_back();
	  writetohof();
	  retval = 1;
	}


    }

  return retval;
}

int Trivia::got_answer(string chatname, string chattext)
{
  if (curstate == STATE_STARTING || curstate == STATE_WAIT_NEW_Q || curstate == STATE_WAITING_FOR_SCORES) return 0; 
  // return immediately if answer has already been gotten
  

  char buf[1000];
  timeval newtime;
  gettimeofday(&newtime, NULL);
  double timeelapsed = newtime.tv_sec + (newtime.tv_usec/1000000.0) - (rawtime.tv_sec + (rawtime.tv_usec/1000000.0));
  sprintf(buf, "***** %s got the right answer in %.2f seconds!", chatname.c_str(), timeelapsed);
  myclient.chatsay (string(buf));  


  double score = chattext.length() * 6.0/timeelapsed;
  sprintf(buf, "***** %s's score is now %.2f points.", chatname.c_str(), update_scorelist(chatname, score));
    // answer length * 6 / timeelapsed
 
  myclient.chatsay(string(buf));
  int a = manage_high_scores(chatname, score, timeelapsed, 0);
  if (a == 1)
    {
      sprintf(buf, "***** %s now has a new record in the Hall Of Fame!", chatname.c_str());
      myclient.chatsay(string(buf));
    }


  if (round_curq > round_numqs)
    {
      myclient.chatsay("*****   That was the final question of the round. Scores coming in 8 seconds."); 
    
      curstate = STATE_WAITING_FOR_SCORES;
      signal(SIGALRM, event_handler);
      alarm(8);
    }
  else
    {
      sprintf(buf, "Question %d of %d coming in a few seconds....", round_curq, round_numqs); 
      myclient.chatsay(string(buf));
      curstate = STATE_WAIT_NEW_Q;
      signal(SIGALRM, event_handler);
      alarm(STATE_WAIT_NEW_Q_TIMER); 

    }
// this should cancel previous alarm, if not try alarm(0)
  return 0;
} 

double Trivia::update_scorelist(string chatname, double score)
{
  for (unsigned int i = 0; i < scorelist.size(); i++)
    {
      if (chatname == scorelist.at(i).name)
	{
	  scorelist.at(i).gen_score += score;
	  return scorelist.at(i).gen_score;
	}
    }
  // not found
  struct Score tempscore;
  tempscore.name = chatname;
  tempscore.gen_score = score;
  scorelist.push_back(tempscore);
  return score;

}
