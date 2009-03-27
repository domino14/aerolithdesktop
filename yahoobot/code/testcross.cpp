#include "crossword.h"
generic_crossword_game mylit;
int main()
{
  mylit.initall();
  mylit.clearmovearray();
   mylit.addtogameboard("FANO", 6, 7, GOING_ACROSS);
  mylit.addtogameboard("ODOMEtRY", 7, 7, GOING_ACROSS);
  mylit.addtogameboard("FOlKIE", 6, 7, GOING_DOWN);
  mylit.addtogameboard("EmBRUTED", 7, 11, GOING_DOWN);
  mylit.addtogameboard("YEARNS", 7, 14, GOING_DOWN);
  mylit.addtogameboard("AMORINO", 4, 4, GOING_ACROSS);

  mylit.changetilerack("HHEED??");
  mylit.generateallplays();

  //mylit.changetilerack("ZZ?ZZZZ");
  // mylit.transposegameboard();
  mylit.printgameboard();
  
  mylit.sortmoves();
  mylit.printmoves();
  cout << mylit.highscoremoves.size() << endl;
  for (int i = 0; i < mylit.highscoremoves.size(); i++)
    if (mylit.mytrie.findword(mylit.highscoremoves.at(i).word) == 0) 
      cout << "BAD " << mylit.highscoremoves.at(i).word << endl;
  //  cout << mylit.findword("TE") << endl;

  //389 moves
}
