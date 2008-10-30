using namespace std;

#include <fstream>

int main()
{
  ofstream o_f;
  o_f.open("client.qrc");
  o_f << "<!DOCTYPE RCC><RCC version=\"1.0\">\n";
  o_f << "<qresource>\n";
  
  for (int i = 1; i <= 73; i++)
    o_f << "\t<file>images/face" << i << ".png</file>\n";

  o_f << "</qresource>\n";
  o_f << "</RCC>\n";

}
