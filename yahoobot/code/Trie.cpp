#include "Trie.h"

vector <Trienode> nodes;

int Trie::findword(const string wordtofind)
{
  int a;
  int parent;
  int wordlength;
  int letterfound;
  int nextsibling;
  int child;
  parent = 0;	
  wordlength = wordtofind.length();
  char thisletter;
  // cout << "(" << wordtofind << ")" << endl;
  for (a = 0; a < wordlength; a++)
    {
      thisletter = toupper(wordtofind[a]);
      letterfound = 0;
      
      child = nodes.at(parent).child;
      //cout << " TEST " << parent << " " << child << endl;
      if (child == NULL_NODE) return 0;
      if (nodes.at(child).letter == thisletter)
	{
	  //	  cout << "letter " << a 
	  //  << "   parent " << parent << " child " << child << endl; 
	  letterfound = 1;
	  parent = child;
	}
      else
	{	//traverse through siblings
	  nextsibling = nodes.at(child).sibling;
	  while (nextsibling != NULL_NODE)
	    {
	      if (nodes.at(nextsibling).letter == thisletter)
		{
		  //    cout << "letter " << a 
		  // << "   child " << child << " sibling " << nextsibling << endl;
		  letterfound = 1;
		  parent = nextsibling;
		}
	      nextsibling = nodes.at(nextsibling).sibling;
	    }
	  if (letterfound == 0) return 0;
	}
      
      
    }
  if (nodes.at(parent).endofword == (char)0) return 0;	
  
  return 1;
}

int Trie::find_partial_word(const string wordtofind)
{
  int a;
  int parent;
  int wordlength;
  int letterfound;
  int nextsibling;
  int foundnode;
  int child;
  parent = 0;	
  
  string oldword;
  oldword = wordtofind;
  wordlength = oldword.length();
  for (a = 0; a < wordlength; a++)
    {
      if (oldword[a] >= 'a' && oldword[a] <= 'z')
	oldword[a] = oldword[a] + 'A' - 'a';
      letterfound = 0;
      child = nodes.at(parent).child;
      if (child == NULL_NODE) return 0;
      if (nodes.at(child).letter == toupper(oldword[a])) 
	{
	  letterfound = 1;
	  parent = child;
	  //	foundnode = nodes[child].child;
	  foundnode = child;
	}
      else
	{	//traverse through siblings
	  nextsibling = nodes.at(child).sibling;
	  while (nextsibling != NULL_NODE)
	    {
	      if (nodes.at(nextsibling).letter == toupper(oldword[a]))
		{
		  letterfound = 1;
		  parent = nextsibling;
		  //	foundnode = nodes[nextsibling].sibling;
		  foundnode = nextsibling;
		}
	      nextsibling = nodes.at(nextsibling).sibling;
	    }
	  if (letterfound == 0) return NULL_NODE;
	}

      
    }
  //if (nodes[parent].endofword == 0) return 0;	
  
  //	return 1;
  return foundnode;
}

void Trie::print_n_nodes(int n)
{
  cout << "letter\tchild\tsibling\teow\n";
  for (int a = 0; a < n; a++)
    {
      cout << a << ": " << nodes[a].letter << "\t" 
	   << nodes[a].child << "\t"
	   << nodes[a].sibling << "\t"
	   << (int)nodes[a].endofword << endl;


    }

}

void Trie::read_wutka_nodes()
{

  unsigned int tempint;
  vector <unsigned int> ints;
  unsigned char bytes[4];
  ifstream i_f;
  i_f.open("dawg.bin", ios::binary | ios:: in);
  while (!i_f.eof())
    {
      i_f.read( (char*) &tempint, 4);
      ints.push_back(tempint);
    }
  i_f.close();
  int a = ints.size();
  cout << endl << "\033[31m" << a << " nodes read." << endl;
   
  int endofnext;
  char myletter, myendofword;
  int mychild, mysibling;
  nodes.push_back(Trienode(NULL_LETTER, 1, ints.size()-27, NULL_NODE));
  
  for (unsigned int i = 1; i < ints.size(); i++)
    {
      bytes[0] = (ints[i] >> 24) & 255;
      bytes[1] = (ints[i] >> 16) & 255;
      bytes[2] = (ints[i] >> 8) & 255;
      bytes[3] = ints[i] & 255;
      myletter = bytes[0];
      if (myletter == 0) myletter = NULL_LETTER;
      else myletter = toupper(myletter);
      myendofword = (unsigned int)(bytes[1]) & 1;
      endofnext = ( (unsigned int)(bytes[1]) >> 1) & 1;
      
      mychild = ( (unsigned int)(bytes[1]) + ( (unsigned int)(bytes[2]) << 8) +
		 ( (unsigned int)(bytes[3]) << 16)) >> 2;
      if (mychild == 0) mychild = NULL_NODE;
      if (endofnext)
	mysibling = NULL_NODE;
      else
	mysibling = i+ 1;
      
      nodes.push_back(Trienode(myletter, myendofword, mychild, mysibling));

    }


}

