#include <format>
#include <iostream>
#include <map>
#include <string>

int
main ()
{
  constexpr uint a = 1;
  constexpr int b = -1;
  constexpr bool r = b < a;
}