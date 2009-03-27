#include "crossword.h"

extern vector <Trienode> nodes;

#if (GAME_TYPE == SCRABBLE)
	
int tileval_g[26] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10};

char gameboard_g[15][16] = {
  
  {"#..2...#...2..#"},
  {".@...3...3...@."},
  {"..@...2.2...@.."},
  {"2..@...2...@..2"},
  {"....@.....@...."},
  {".3...3...3...3."},
  {"..2...2.2...2.."},
  {"#..2...*...2..#"},
  {"..2...2.2...2.."},
  {".3...3...3...3."},
  {"....@.....@...."},
  {"2..@...2...@..2"},
  {"..@...2.2...@.."},
  {".@...3...3...@."},
  {"#..2...#...2..#"} };

#endif
#if (GAME_TYPE == LITERATI)
int tileval_g[26] = {1, 2, 1, 1, 1, 3, 1, 2, 1, 5, 3, 1, 1, 1, 1, 2, 5, 1, 1, 1, 1, 4, 4, 5, 3, 5};
char gameboard_g[15][16] = {			//[# rows][# columns]
  { "...#..3.3..#..."},
  { "..2..@...@..2.."},
  { ".2..2.....2..2."},
  { "#..3...@...3..#"},
  { "..2...2.2...2.."},
  { ".@...3...3...@."},
  { "3...2.....2...3"},
  { "...@...*...@..."},
  { "3...2.....2...3"},
  { ".@...3...3...@."},
  { "..2...2.2...2.."},
  { "#..3...@...3..#"},
  { ".2..2.....2..2."},
  { "..2..@...@..2.."},
  { "...#..3.3..#..."} };

#endif

// moves will be sorted by Score

bool operator<(const Move& x, const Move& y)
{
  return x.weightedscore < y.weightedscore;
}
/////////////////////////////////////
int generic_crossword_game::sortmoves()
{

  sort(highscoremoves.begin(), highscoremoves.end());
  cout << "Sorted " << highscoremoves.size() - 1 << " moves." << endl;
  return (int)(highscoremoves.size() -1);
}
///////////////////
void generic_crossword_game::printmoves()
{
  for (unsigned int i = 0; i < highscoremoves.size(); i++)
    highscoremoves[i].printmove();
}

void generic_crossword_game::printtilerack()
{
  for (int i = 0; i < 27; i++)
    cout << tilerack[i];
  cout << endl;
}

void generic_crossword_game::changetilerack(string newtiles)
{
  for (unsigned int i = 0; i < newtiles.size(); i++)
    {
      if (newtiles[i] >='A' && newtiles[i] <= 'Z')
	tilerack[newtiles[i]-'A']++;
      if (newtiles[i] == '?')
	tilerack[26]++;

    }

}

void generic_crossword_game::addtogameboard(string wordtoadd, int r, int c, int dir)
{
  if (dir == GOING_DOWN)
    {
      for (unsigned int i = r, iter = 0; i < r + wordtoadd.size(); i++, iter++)
	gameboard[i][c] = wordtoadd[iter];
    }
  if (dir == GOING_ACROSS)
    {
      for (unsigned int j = c, iter = 0; j < c + wordtoadd.size(); j++, iter++)
	gameboard[r][j] = wordtoadd[iter];
    }
}

void generic_crossword_game::clearmovearray()
{
  highscoremoves.clear();
}

void generic_crossword_game::transposegameboard()
{
  char temp[15][15];
  int i, j;
  
  for (i = 0; i < 15; i++)
    {
      for (j = 0; j < 15; j++)
	{
	  temp[i][j] = gameboard[i][j];
	}
    }
  
  for (i = 0; i < 15; i++)
    {
      for (j = 0; j < 15; j++)
	{
	  gameboard[i][j] = temp[j][i];
	}
    }
}

void generic_crossword_game::printgameboard()
{
  int i, j;
  
  for (i = 0; i < 15; i++)  // row = y
    {
      for (j = 0; j < 15; j++)  // column = x
	{
	  cout << gameboard[i][j]; // prints out row by row
	}
      cout << endl;
    }
}

void generic_crossword_game::initall()
{
  twotothe[0] = 1;
  for (int a = 1; a < 26; a++)
    twotothe[a] = 2 * twotothe[a-1];
  mytrie.read_wutka_nodes();
  cout << "Added all words. " << nodes.size() << " nodes created.\033[0m "<<  endl;
  clearmovearray();
  // copy tileval_g to tileval
  for (int i = 0; i < 26; i++)
    tileval[i] = tileval_g[i];

  clear_gameboard();

  for (int i = 0; i < 26; i++)
    tilerack[i] = 0;
}

void generic_crossword_game::clear_gameboard()
{
  for (int i = 0; i < 15; i++)
    for (int j = 0; j < 15; j++)
      gameboard[i][j] = gameboard_g[i][j];
}

void generic_crossword_game::generateallplays()
{
  int row_number;
  string row_contents(16, ' ');
  int i, j;
  int ytop, ybottom;
  int stopped_scanning;
  string testword;
  int x;
  int testchar;
  long cross_checks[16];
  short rowanchors[16];
  int cross_sums[16];
  clearmovearray();
  resetvectors();
  computeanchors();
  
  for (i = 0; i < 15; i++)
    {
      for (j = 0; j < 15; j++)
	{
	  if (anchors[i][j] == 1) 
	    {
	      
	      // words read vertically
	      // scan up and scan down for limit
	      //cesstrcpy(testword, "               ");
	      testword = "";
	      stopped_scanning = 0;
	      ytop = i;
	      while (stopped_scanning == 0)
		{
		  ytop = ytop - 1;
		  if (ytop < 0) stopped_scanning = 1;
		  else
		    {
		      if (gameboard[ytop][j] < 65) stopped_scanning = 1;	
		      // empty
		    }
		}
	      ytop++;
	      //	if (ytop == j) ytop++;	// there was no top part
	      
	      stopped_scanning = 0;
	      ybottom = i;
	      while (stopped_scanning == 0)
		{
		  ybottom = ybottom + 1;
		  if (ytop > 14) stopped_scanning = 1;
		  else
		    {
		      if (gameboard[ybottom][j] < 65) stopped_scanning = 1;	
		      // empty
		    }
		}
	      ybottom--;
	      //	if (ybottom == j) ybottom--;	// there was no bottom part
	      
	      if (ytop == ybottom) 
		// there was no top or bottom part. anchor was horizontal
		{
		}
	      else
		{
		  crosscheckvectors[i][j] = 0;
		  for (x = ytop; x < i; x++)
		    {
		      //testword[curchar] = toupper(gameboard[x][j]);
		      testword += toupper(gameboard[x][j]);
		      if (gameboard[x][j] <= 'Z' && gameboard[x][j] >= 'A') 
			crosssums[i][j] += tileval[gameboard[x][j] - 'A'];
		      // else add 0
		    }
		  testword += NULL_LETTER;
		  testchar = testword.size() - 1;
		  for (x = i + 1; x <= ybottom; x++)
		    {
		      //testword[curchar] = toupper(gameboard[x][j]); 
		      //this is the character we are testing!
		      testword += toupper(gameboard[x][j]);
		      if (gameboard[x][j] <= 'Z' && gameboard[x][j] >= 'A') 
			crosssums[i][j] += tileval[gameboard[x][j] - 'A'];
		    }
		  for (x = 'A'; x <= 'Z'; x++)
		    {
		      testword[testchar] = x;
		      if (mytrie.findword(testword) == 1)
			{
			  crosscheckvectors[i][j] += 1 << (x - 'A');
			  // cout << "before POSSIBLE WORD " << testword << endl;
			}
		    }
		  
		}
	      
	    }
	}
    }
  
  
  for (row_number = 0; row_number < 15; row_number++)
    {
      //      cesstrcpy(row_contents, gameboard[row_number]);
      //      row_contents = gameboard[row_number]; // possible memory leak
      for (int ii = 0; ii < 15; ii++)
	row_contents[ii] = gameboard[row_number][ii];
      for (i = 0; i < 15; i++)
	{
	  cross_checks[i] = crosscheckvectors[row_number][i];
	  rowanchors[i] = anchors[row_number][i];
	  cross_sums[i] = crosssums[row_number][i];
	}
      
      cross_checks[15] = 0;
      generate_row_plays(row_contents, cross_checks, rowanchors, row_number, cross_sums);
      
      
    }
  transposegameboard();
  
  for (i = 0; i < 15; i++)
    {
      for (j = 0; j < 15; j++)
	{
	  if (anchors[j][i] == 1) 
	    {
	      
	      // words read vertically
	      // scan up and scan down for limit
	      //	      cesstrcpy(testword, "               ");
	      testword = "";
	      stopped_scanning = 0;
	      ytop = i;
	      while (stopped_scanning == 0)
		{
		  ytop = ytop - 1;
		  if (ytop < 0) stopped_scanning = 1;
		  else
		    {
		      if (gameboard[ytop][j] < 65) stopped_scanning = 1;	// empty
		    }
		}
	      ytop++;
	      //	if (ytop == j) ytop++;	// there was no top part
	      
	      stopped_scanning = 0;
	      ybottom = i;
	      while (stopped_scanning == 0)
		{
		  ybottom = ybottom + 1;
		  if (ytop > 14) stopped_scanning = 1;
		  else
		    {
		      if (gameboard[ybottom][j] < 65) stopped_scanning = 1;	// empty
		    }
		}
	      ybottom--;
	      //	if (ybottom == j) ybottom--;	// there was no bottom part
	      
	      if (ytop == ybottom) // there was no top or bottom part. anchor was horizontal
		{
		}
	      else
		{
		  //		  curchar = 0;
		  crosscheckvectorstrans[i][j] = 0;
		  for (x = ytop; x < i; x++)
		    {
		      //  testword[curchar] = toupper(gameboard[x][j]);
		      testword += toupper(gameboard[x][j]);
		      if (gameboard[x][j] <= 'Z' && gameboard[x][j] >= 'A') 
			crosssumstrans[i][j] += tileval[gameboard[x][j] - 'A'];
		      //curchar++;
		    }
		  testword += NULL_LETTER;
		  testchar = testword.length() - 1;
		  for (x = i + 1; x <= ybottom; x++)
		    {
		      //testword[curchar] = toupper(gameboard[x][j]); 
		      testword += toupper(gameboard[x][j]);
		      if (gameboard[x][j] <= 'Z' && gameboard[x][j] >= 'A') 
			crosssumstrans[i][j] += tileval[gameboard[x][j] - 'A'];
		      // curchar++;
		    }
		  //testword[curchar] = '\0';
		  
		  for (x = 'A'; x <= 'Z'; x++)
		    {
		      testword[testchar] = x;
		      if (mytrie.findword(testword) == 1) 
			{
			  crosscheckvectorstrans[i][j] += 1 << (x - 'A');
			//	cout << "after POSSIBLE WORD " << testword << endl;
			}
		    }
		  
		}
	      
	    }
	}
    }
  
  for (row_number = 100; row_number < 115; row_number++)
    {
      //      cesstrcpy(row_contents, gameboard[row_number - 100]);
      for (int ii = 0; ii < 15; ii++)
	row_contents[ii] = gameboard[row_number - 100][ii];      
      for (i = 0; i < 15; i++)
	{
	  cross_checks[i] = crosscheckvectorstrans[row_number - 100][i];
	  rowanchors[i] = anchors[i][row_number - 100];
	  cross_sums[i] = crosssumstrans[row_number - 100][i];
	}
      
      cross_checks[15] = 0;
      generate_row_plays(row_contents, cross_checks, rowanchors, row_number, cross_sums);
      
    }
  
  transposegameboard();
}

void generic_crossword_game::generate_row_plays(const string row_contents, const long cross_checks[], const short anchors[], const short row_number, const int cross_sums[])
{
  int k;
  // anchors has dimension 15;
  int a;
  int lastanchor;
  int i;
  int j;
  int mynode;
  int wordlen;
  string PartialWord;
  
  lastanchor = -1;
  for (a = 0; a < 15; a++)
    {
      PartialWord = "";
      if (anchors[a] == 1)
	{
	  k = a - lastanchor - 1;
	  lastanchor = a;
	  if (a > 0)
	    {
	      if (row_contents[a - 1] >= 65) //there is a tile to the left of anchor square
		{
		  
		  for (i = a - 1, wordlen = 0; i >= 0; i--, wordlen++)
		    {	
		      if (row_contents[i] < 65) break;
		    }
		  
		  // wordlen is length of word already on board
		  
		  for (j = 0; j < wordlen; j++)
		    {	
		      PartialWord += row_contents[a - wordlen + j];
		    }

		  mynode = mytrie.find_partial_word(PartialWord);
		  
		  if (mynode != NULL_NODE) 
		    ExtendRight(PartialWord, mynode, a, row_contents, cross_checks, a, row_number, 0, cross_sums, anchors);
		}
	      else 
		LeftPart(PartialWord, 0, k, a, row_contents, cross_checks, row_number, 0, cross_sums, anchors); 
	    }
	  if (a == 0) 
	    LeftPart(PartialWord, 0, k, a, row_contents, cross_checks, row_number, 0, cross_sums, anchors);
	  
	}
    }
  
}


// i am here
void generic_crossword_game::LeftPart(const string PartialWord, int node, int limit, int AnchorSquare, const string row_contents, const long cross_checks[], const short row_number, short num_tiles_used, const int cross_sums[], const short anchors[])
{
  int nextsibling;
  char nextletter;
  string newword, oldword;
  newword = PartialWord;
  if (node == NULL_NODE) return;
  ExtendRight(newword, node, AnchorSquare, row_contents, cross_checks, AnchorSquare, row_number, num_tiles_used, cross_sums, anchors);

  if (limit > 0)
    {
      nextsibling = nodes.at(node).child;
      if (nextsibling == NULL_NODE) return;
      nextletter = nodes.at(nextsibling).letter;
      while (nextletter != NULL_LETTER)
	{
	  if (tilerack[(int)(nextletter - 'A')] >= 1)	// nextletter is in rack!
	    {
	      tilerack[(int)(nextletter - 'A')] -= 1;
	      
	      oldword = newword;
	      newword += nextletter;
	      LeftPart(newword, nextsibling, limit - 1, AnchorSquare, row_contents, cross_checks, row_number, num_tiles_used + 1, cross_sums, anchors);
	      newword = oldword;
	      tilerack[(int)(nextletter - 'A')] += 1;	// put tile back
	    }

	  if (tilerack[26] >= 1) 
	    // there is a blank in the rack, let it represent next letter!
	    {
	      tilerack[26] -=1;
	      oldword = newword;
	      newword += (nextletter + 'a' - 'A');
	      LeftPart(newword, nextsibling, limit - 1, AnchorSquare, row_contents, cross_checks, row_number, num_tiles_used + 1, cross_sums, anchors);
	      newword = oldword;
	      tilerack[26] += 1;	// put tile back
	      
	    }
	  
	  nextsibling = nodes.at(nextsibling).sibling;
	  if (nextsibling == NULL_NODE)
	    {
	      nextletter = NULL_LETTER;
	    }
	  else
	    {
	      nextletter = nodes.at(nextsibling).letter;
	    }
	}
    }
}

void generic_crossword_game::ExtendRight(const string PartialWord, int node, int Square, const string row_contents, const long cross_checks[], int anchor, const short row_number, short num_tiles_used, const int cross_sums[], const short anchors[])
{
  char nextletter;
  int nextsibling = node;
  int next_square;
  char checkletter;
  string newword, oldword;
  char oldletter;
  newword = PartialWord;
  if (node == NULL_NODE) return;
  if (Square < 16) // or 16, i dont know??
    {
      //      if (Square == 15) cout << "Square = 15" << endl;
      // if square is vacant then
      if (row_contents[Square] < 65)
	{
	  //  if (Square == 15) cout << "cont " << row_contents[Square] << endl;
	  if (nodes.at(node).endofword == (char)1 && Square > anchor)	
	    {
	      
	      LegalMove(PartialWord, Square - 1, row_number, row_contents,  num_tiles_used, cross_sums, anchors);
	    }
	  // for each edge E out of N
	  
	  nextsibling = nodes.at(node).child;
	  if (nextsibling == NULL_NODE) return;
	  nextletter = nodes.at(nextsibling).letter;
	  while (nextletter != NULL_LETTER)
	    {
	      if ( (cross_checks[Square] & twotothe[(nextletter - 'A')]) == twotothe[(nextletter - 'A')])	// passes cross-checks
		{
		  if (tilerack[(int)(nextletter - 'A')] >= 1) 	// nextletter is in rack!
		    {
		      tilerack[(int)(nextletter - 'A')] -= 1;
		      
		      next_square = Square + 1;
		      oldword = newword;
		      newword += nextletter;
		      ExtendRight(newword, nextsibling, next_square, row_contents, cross_checks, anchor, row_number, num_tiles_used + 1, cross_sums, anchors);
		      newword = oldword;
		      tilerack[(int)(nextletter - 'A')] += 1;	// put tile back
		    }
		  if (tilerack[26] >= 1)
		    {
		      tilerack[26] -= 1;
		      next_square = Square + 1;
		      oldword = newword;
		      newword += (nextletter + 'a' - 'A');
		      ExtendRight(newword, nextsibling, next_square, row_contents, cross_checks, anchor, row_number, num_tiles_used + 1, cross_sums, anchors);
		      newword = oldword;
		      tilerack[26] += 1;
		    }
		  
				
				
		}
	      
	      nextsibling = nodes.at(nextsibling).sibling;
	      if (nextsibling == NULL_NODE)
		{
		  nextletter = NULL_LETTER;
		}
	      else
		{
		  nextletter = nodes.at(nextsibling).letter;
		}
	      
	      
	    }
	  
	}
      else	// if the square is not vacant
	{
	  nextletter = row_contents[Square];
	  oldletter = nextletter;
	  if (nextletter >= 'a' && nextletter <= 'z') // between 'a' and 'z'
	    nextletter = toupper(nextletter);
	  nextsibling = nodes.at(node).child;
	  if (nextsibling == NULL_NODE) 
	    checkletter = NULL_LETTER;
	  else
	    checkletter = nodes.at(nextsibling).letter;
	  while (checkletter != NULL_LETTER)
	    {
	      if (nextletter == checkletter)
		{
		  next_square = Square + 1;
		  oldword = newword;
		  newword += oldletter;
		  ExtendRight(newword, nextsibling, next_square, row_contents, cross_checks, anchor, row_number, num_tiles_used, cross_sums, anchors);
		  newword = oldword;
		}
	      nextsibling = nodes.at(nextsibling).sibling;
	      if (nextsibling == NULL_NODE)
		{
		  checkletter = NULL_LETTER;
		}
	      else
		{
		  checkletter = nodes.at(nextsibling).letter;
		}
	    }
	  
	  
	}
      

    }
}

void generic_crossword_game::LegalMove(string PartialWord, int square, short row_number, string row_contents, const short num_tiles_used, const int cross_sums[], const short anchors[])
{
  int i;
  int j = 0;
  //word ends at square
  char strplay[3];
  strplay[2] = '\0';

  string oldword;
  int myscore = 0;
  int wordlen;
  int scoretoadd = 0;
  int tilescore;
  int thiswordscore = 0;
  oldword = PartialWord;
  char tiletoplace;
  int multiplier = 1;
  int usedtriple = 0;
  i = square + 1 - oldword.length();

  if (row_number >= 100) // vertical
    {
      
      strplay[0] = row_number - 100 + 'a';
      strplay[1] = i + 'A';
    }
  else
    {
      //horizontal
      strplay[0] = row_number + 'A';
      strplay[1] = i + 'a';
    }
  
  
  // check cross_sums for score.
	// cross_sums for a square that is already full at computation time should be the value for that tile
	// for empty squares it's the value of the sum of the values of all the tiles in contiguous sequence above and below that square
  wordlen = oldword.length();
	
  string tilesiplace;
	// calculate horizontally then vertically
	for (j = i; j < i + wordlen; j++)
	  {
	    if (row_contents[j] < 65)	
	      // then we are placing tile here	'it's either @, #, 2, 3, *, or .
	      {
		tiletoplace = oldword[j - i];
		tilesiplace += tiletoplace;
		if (tiletoplace >= 'a' && tiletoplace <= 'z') tilescore = 0;
		else tilescore = tileval[tiletoplace - 'A'];
		
		if (row_contents[j] == '2') tilescore = tilescore * 2;
		if (row_contents[j] == '3') tilescore = tilescore * 3;
		if (row_contents[j] == '*' && GAME_TYPE == SCRABBLE) multiplier *= 2;
		if (row_contents[j] == '@') multiplier *= 2;
		if (row_contents[j] == '#') 
		  {
		    multiplier *= 3;
		    usedtriple = 1;
		  }
		
		thiswordscore += tilescore;
		// count bonus squares once if wordlength is one, else count twice
	      }
	    else
	      {
		// there's a tile already here. add its value to thiswordscore
		tiletoplace = row_contents[j];
		if (tiletoplace >= 'a' && tiletoplace <= 'z')  ; //thiswordscore += 0;
		else thiswordscore += tileval[tiletoplace - 'A'];
	      }
	    //	    o_f2 << "J, I " << j << " " << i << " " << thiswordscore << endl;
	  }
	
	thiswordscore *= multiplier;
	myscore += thiswordscore;
	//o_f2 << "My Horizontal Score " << myscore << endl;
	thiswordscore = 0;
	// now calculate vertically
	for (j = i; j < i + wordlen; j++)
	  {
	    if (row_contents[j] < 65 && anchors[j] == 1) 
	      /* && cross_sums[j] > 0*/	// then we are placing tile here	'it's either @, #, 2, 3, *, or .
	      {
		if (is_vertical_anchor(row_number, j))
		  {
		    // must be a vertical anchor
		    
		    tiletoplace = oldword[j - i];
		    if (tiletoplace >= 'a' && tiletoplace <= 'z') tilescore = 0;
		    else tilescore = tileval[tiletoplace - 'A'];
		    
		    scoretoadd = cross_sums[j];
		    if (row_contents[j] == '.') 
		      thiswordscore += tilescore + scoretoadd;
		    if (row_contents[j] == '2') 
		      thiswordscore += (tilescore * 2) + scoretoadd;
		    if (row_contents[j] == '3') 
		      thiswordscore += (tilescore * 3) + scoretoadd;
		    if (row_contents[j] == '@') 
		      thiswordscore += (tilescore + scoretoadd) * 2;
		    if (row_contents[j] == '#') 
		      {
			thiswordscore += (tilescore + scoretoadd) * 3;
			usedtriple = 2;
		      }
			// count bonus squares once if wordlength is one, else count twice
		    //o_f2 << "J, I " << j << " " << i << " " << thiswordscore << endl;
		  }
	      }
	    // if we're not placing a tile, there is no vertical score to calculate
	  }
	myscore += thiswordscore;
	if (num_tiles_used == 7) myscore += BINGO_BONUS;
	
	//o_f2 << oldword << " " << wordlen <<  " " << strplay << " " << myscore << "\n";
  // calculate rack leave.
	string rackleave;
	for (int i = 0; i < 27; i++)
	  if (tilerack[i] > 0)
	    {
	      if (i != 26)
		for (int j = 0; j < tilerack[i]; j++)
		  rackleave += (char)i + 'A';
	      else
		for (int j = 0; j < tilerack[26]; j++)
		  rackleave += '?';
	    }
	int myweightedscore = 0;
	myweightedscore = apply_heuristics(oldword, myscore, usedtriple, 
					   tilesiplace, rackleave);

	//	cout << oldword << endl;
	//cout << strplay << endl;
	//cout << myscore << endl;
	//cout << myweightedscore << endl;
	//cout << rackleave << endl;
	//cout << tilesiplace + rackleave << endl;
	Move tempmove(oldword, strplay, myscore, myweightedscore, 
		      rackleave, tilesiplace + rackleave);
	//tempmove.printmove();
	//	printtilerack();
	highscoremoves.push_back(tempmove);
}

int generic_crossword_game::apply_heuristics(string myword, int myscore, 
					     int usedtriple, string tilesiplace, string rackleave)
{
  // here we have some heuristics.
  // tilerack
  // first of all, we don't want to use blanks like dumbasses
  // take away 7 points per blank
  int retval = myscore;
  int dontsave = 0;
  for (unsigned int i = 0; i < tilesiplace.length(); i++)
    if (tilesiplace.at(i) >= 'a') // lowercase, means we used a blank
      retval -= 10;

  

  retval += 5*usedtriple; 
  // used a triple, therefore denying the opponent of it

  // make sure we don't expose 3w's
  // im not sure how to write a penalty for leaving a triple open yet. think of this 
  // later


  // now weigh rack leave accordingly

  if (rackleave.length() == 0 && tilesiplace.length() == 7)
    {
      //played abingo
      return retval;
    }

  if (tilesiplace.length() + rackleave.length() < 7)
    return retval + tilesiplace.length(); // try to go out if possible

  // don't save up good tiles if we can't get a bingo anymore!


    // lose Q, Z and J!

  unsigned int findq = rackleave.find("Q", 0);
  unsigned int findu = rackleave.find("U", 0);
  if (findq != string::npos && findu != string::npos)
    { // nothing, no weight
    }
  else if (findq != string::npos)
    {
      // q without u BAD
      retval -= 7;
    }

  if (rackleave.find("Z", 0) != string::npos)
    retval -= 4; // Z and J not that bad
  if (rackleave.find("J", 0) != string::npos)
    retval -= 4;

  // vowel consonant balance.
  int numvowels = 0, numconsonants = 0;
  for (unsigned int i = 0; i < rackleave.length(); i++)
    {
      if (rackleave.at(i) == 'A' || rackleave.at(i) == 'E' || rackleave.at(i) == 'I'
	  || rackleave.at(i) == 'O' || rackleave.at(i) == 'U')
	numvowels++;
      else if (rackleave.at(i) == '?')
	// nothing
	;
      else // consonants
	numconsonants++;
    }
  int diff = numconsonants - numvowels;
  if (diff == 0)
    retval += 2;
  if (diff == 1)
    retval -= 1;
  if (diff > 1)
    retval -= 2 * (diff - 1);
  if (diff == -1)
    retval -= 2;
  if (diff < - 1)
    retval -= 3 * (-diff - 1);

  // saving up good tiles
  
  if (dontsave) return retval;

  unsigned int founde = rackleave.find("E", 0);
  if (founde != string::npos && rackleave.find("R", 0) != string::npos
      && rackleave.find("S", 0) != string::npos)
    retval += 5;
  else if (rackleave.find("I", 0) != string::npos && rackleave.find("N", 0) != string::npos && rackleave.find("G", 0) != string::npos)
    retval +=4;
  else if (founde != string::npos && rackleave.find("R", 0) != string::npos)
    retval += 3;
  else if (founde != string::npos && rackleave.find("S", 0) != string::npos)
    retval += 3;
  else if (founde != string::npos && rackleave.find("D", 0) != string::npos)
    retval += 3;
  else if (founde != string::npos && rackleave.find("S", 0) != string::npos &&
	   rackleave.find("I", 0) != string::npos)
    retval += 4;
  return retval;

}

void generic_crossword_game::computeanchors()
{
  int i, j;
  int numanchors = 0;
  // compute cross checks and anchors for every square
  for (i = 0; i < 15; i++)
    {
      for (j = 0; j < 15; j++)
	{
	  anchors[i][j] = 0;
	  if (gameboard[i][j] < 'A')
	    {

	      if (i > 0)
		{
		  if (gameboard[i - 1][j] >= 'A')	
		    {
		      anchors[i][j] = 1;
		      numanchors++;
		    }
		}
	      if (i < 14)
		{
		  if (gameboard[i + 1][j] >= 'A') 
		    {
		      anchors[i][j] = 1;
		      numanchors++;
		    }
		}
	      if (j > 0)
		{
		  if (gameboard[i][j - 1] >= 'A')	
		    {
		      anchors[i][j] = 1;
		      numanchors++;
		    }
		}
	      if (j < 14)
		{
		  if (gameboard[i][j + 1] >= 'A')	
		    {
		      anchors[i][j] = 1;
		      numanchors++;
		    }
		}
	      
	    }
	  
		
	}
    }
  
  if (numanchors == 0) anchors[7][7] = 1;	
}

void generic_crossword_game::resetvectors()
{
  int a, x;
  for (a = 0; a < 16; a++)
    {
      for (x = 0; x < 16; x++)
	{
	  crosscheckvectors[a][x] = TRIVIAL_CROSS_CHECK;
	  crosscheckvectorstrans[a][x] = TRIVIAL_CROSS_CHECK;
	  crosssums[a][x] = 0;
	  crosssumstrans[a][x] = 0;
	}
    }
}

int generic_crossword_game::is_vertical_anchor(const int row_number, const int square)
{
  int row = row_number;
  if (row_number >= 100)
    {
      row -= 100;
    }
  if (row > 0)
    {
      if (gameboard[row - 1][square] >= 65) return 1;
    }
  if (row < 14)
    {
      if (gameboard[row + 1][square] >= 65) return 1;
    }
  return 0;
}
