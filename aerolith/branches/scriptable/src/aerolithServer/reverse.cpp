//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
  if (argc !=2)
    {
      cout << "Usage: reverse filename\n";
      return 0;
    }
  ifstream i_f;
  i_f.open(argv[1]);
  while (!i_f.eof())
    {
      char line[256];
      i_f.getline(line, 256);
      if (strlen(line) > 1)
	{
	  for (int i = strlen(line) - 1; i >= 0; i--)
	    {
	      cout << line[i];
	    }
	  cout << endl;

	}
  
    }

}
