using namespace std;
#include <string>
#include <iostream>
#include <vector>


int main(int argc, char* argv[])
{
  unsigned int i, x;
  unsigned int b;
  unsigned char nextbyte;
  string cmd;
  vector <unsigned int> myints;

  if (argc == 1)
    {
      cout << "Usage: " << endl;
      cout << "decoder fval b1 b2 ... bn" << endl;
      cout << "b1 MUST correspond to ascii value fval" << endl;
      return 0;
    }
  else
    {
      x = strtol(argv[2], NULL, 16);
     
      b = (unsigned char)x ^ atoi(argv[1]);
      nextbyte = b * 83;
      myints.push_back(atoi(argv[1]));
      for (i = 3; i < argc; i++)
	{
	  x = strtol(argv[i], NULL, 16);
	  b = (unsigned char)x ^ (unsigned char)nextbyte;
	  myints.push_back(b);
	  nextbyte *= 83;;
	}
      cout << endl;
    }

  for (i = 0; i < myints.size(); i++)
    {
      cout << myints[i] << " ";
    }
  cout << endl;

  cmd.resize(myints.size());
  for (i = 0; i < myints.size(); i++)
    {
      cmd[i] = char(myints[i]);
    }
  cout << cmd << endl;
  //  mycmd.ProcessCommand(cmd);
  return 0;
}
