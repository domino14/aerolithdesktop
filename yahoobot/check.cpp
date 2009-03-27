using namespace std;

#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
  string cmd = "grep -i ";
  if (argc != 2) 
    return 0;
  else
    {
      cmd += argv[1];
      cmd += " words.txt";
      system(cmd.c_str());
    }
  return 0;
}
