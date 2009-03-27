#ifndef _TRIVIA_H_
#define _TRIVIA_H_
using namespace std;
#include <string>
#include <vector>
#include <sys/time.h>
#include <map>

typedef enum
  {
    STATE_STARTING,
    STATE_WAIT_NEW_Q,
    STATE_IN_Q_NO_HINT,
    STATE_IN_Q_AFTER_HINT,
    STATE_WAITING_FOR_SCORES,
    STATE_OFF
  } trivia_state;

//seconds
#define STATE_STARTING_TIMER 30
#define STATE_WAIT_NEW_Q_TIMER 15
#define STATE_IN_Q_NO_HINT_TIMER 15
#define STATE_IN_Q_AFTER_HINT_TIMER 20

typedef struct Score
{
  double gen_score;  // a generic score, can be reused for different definitions
  string name;
} score;

typedef struct Timescore
{
  double time;
  string name;
} timescore;


class Trivia
{
 private:

 public:
  static int numqs;
  static vector <string> questions;
  static vector <string> answers;
  static trivia_state curstate;
  static timeval rawtime;
  static string curq, cura, curh;
  static int init();
  static int get_qs();
  static int NewQuestion();
  static int start();
  static void event_handler(int sig);
  static int got_answer(string chatname, string chattext);
  static double update_scorelist(string chatname, double score);
  static int displayscores();
  static int get_num_words();
  static vector <score> scorelist;
  static int round_numqs;
  static int round_curq;
   
};

#endif
