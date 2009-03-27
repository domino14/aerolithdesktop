using namespace std;

#include <inttypes.h>
#include <iostream>

unsigned char x[10] = {-1, 1, 127, 128, 129, 255, 240, 150, -100, -150};

int main()
{
  for (int a = 0; a < 10; a++)
    cout << ((uint32_t)(x[a])) << endl;
}
