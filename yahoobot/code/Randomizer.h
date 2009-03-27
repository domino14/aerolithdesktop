#ifndef _RANDOMIZER_H_
#define _RANDOMIZER_H_

/* following are taken directly from decompiled Java class 
_cls135.jad and translated for C++, which means
barely any translation at all
Function names are a bit cryptic but whatever.*/
#include <inttypes.h>

class rand_class
{
public:
  // constructor
  rand_class(uint64_t l)
  {
    Np(l);
    //    a = 0;
  }
  
  //blank constructor
  rand_class()
  {
  }

  void seed (uint64_t l)
  {
    Np(l);
  }

  uint32_t Jp(int i)
    {
    uint64_t el = a * (uint64_t)0x5deece66dLL + (uint64_t)11 & (uint64_t)0xffffffffffffLL;
    a = el;
    return (uint32_t) (el >> 48 - i);
  }
  
  uint32_t Lp()
  {
    return Jp(32);
  }
  
  uint32_t Ip(int i)
  {
    return (Lp() & 0x7fffffff) % i;
  }

  void Np(uint64_t el)
  {
    a = (el ^ (uint64_t)0x5deece66dLL) & (uint64_t)0xffffffffffffLL;
  }

private:
  uint64_t a;
};

#endif
