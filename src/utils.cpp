#include "utils.h"

bool isOdd(int val) {
  return (val % 2 == 1);
}

bool isEven(int val) {
  return (val % 2 == 0);
}

bool isDivisable(int dividend, int divisor) {
  return ((dividend % divisor) == 0);
}

int findNextDivisable(int dividend, int divisor) {
  int rem = dividend % divisor;
  return (dividend - rem);
}
