#ifndef _CROSSWORD_H_
#define _CROSSWORD_H_
using namespace std;
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "Trie.h"
#define TRIVIAL_CROSS_CHECK	67108863	
// every letter is allowable. only happens on the first move, and then on invalid rows

#define GOING_DOWN 0
#define GOING_ACROSS 1

// implement dawg as a 26-ary tree

#define SCRABBLE 1
#define LITERATI 0

#define GAME_TYPE LITERATI
#define BINGO_BONUS 35 + (15 * GAME_TYPE)

#define max(A, B)  ((A) > (B) ? (A) : (B))

typedef struct mode
{
  char lowball;
  char tens;
  char twelves;
  char continuous;
  char lit;
  char rated;
  char trivia;
} Gamemode;


class Move
{
 public:
  string word;
  string strplay;
  string rackleave;
  int score;
  int weightedscore;
  string rack;

  Move(string newword, string newstrplay, int newscore, 
       int newweightedscore, string newrackleave, string newrack)
    {
      word = newword;
      strplay = newstrplay;
      score = newscore;
      weightedscore = newweightedscore;
      rackleave = newrackleave;
      rack = newrack;
    }
  void printmove()
    {
      cout << word << ", "<< strplay <<", " << score << ", " << weightedscore 
	   << "\t(" << rackleave << ")\t(" << rack << ")";
      cout << endl;
    }
};

class generic_crossword_game
{
public:
  void initall();
  void clear_gameboard();
  void generate_row_plays(const string row_contents, const long cross_checks[], const short anchors[], const short row_number, const int cross_sums[]);
  void LeftPart(const string PartialWord, int node, int limit, int AnchorSquare, const string row_contents, const long cross_checks[], const short row_number, short num_tiles_used, const int cross_sums[], const short anchors[]);
  void ExtendRight(const string PartialWord, int node, int Square, const string row_contents, const long cross_checks[], int anchor, const short row_number, short num_tiles_used, const int cross_sums[], const short anchors[]);
  void LegalMove(const string PartialWord, int square, short row_number, const string row_contents, const short num_tiles_used, const int cross_sums[], const short anchors[]);
  int is_vertical_anchor(const int row_number, const int square);
  void clearmovearray();
  void transposegameboard();
  void printgameboard();
  void generateallplays();
  void computeanchors();
  void resetvectors();
  void addtogameboard(string wordtoadd, int r, int c, int dir);
  void changetilerack(string newtiles);
  void printmoves();
  void printtilerack();
  int sortmoves();
  int apply_heuristics(string myword, int myscore, int usedtriple, string tilesiplace, string rackleave);

// probably should write accessor functions for these but whatever
  vector <Move> highscoremoves;
  vector <char> tiles;
  short tilerack[27];
  // THERE IS A MEMORY LEAK HERE!!!
  char gameboard[16][16];
  Trie mytrie;
  //vector <Treenode> nodes;
private:
  long crosscheckvectors[16][16];
  long crosscheckvectorstrans[16][16];
  int anchors[15][15];
  int crosssums[16][16];
  int crosssumstrans[16][16];
  long twotothe[26];
  int tileval[26];
  
};

#endif
