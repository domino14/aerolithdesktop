#ifndef _TRIE_H_
#define _TRIE_H_
using namespace std;
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#define NULL_NODE (1 << 19) - 1
#define NULL_LETTER '['


class Trienode
{
 public:
  char letter;
  char endofword;
  int child;
  int sibling;
  char processed;
  
  Trienode(char newlet, char newend, int newchild, int newsibling)
    {
      letter = newlet;
      endofword = newend;
      child = newchild;
      sibling = newsibling;
      //  mynumber = newnumber;
    }
  
  Trienode()
    {
      //empty
      letter = NULL_LETTER;
      endofword = (char)0;
      child = NULL_NODE;
      sibling = NULL_NODE;
      //mynumber = -1;
    }
};

class Trie
{
 public:
  void print_n_nodes(int n);
  int findword(const string wordtofind);
  void createtree(int readfromwordfile, int writetonodefile);
  int find_partial_word(const string wordtofind);
  void read_wutka_nodes();
};

#endif

