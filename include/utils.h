#ifndef UTILS_H
#define UTILS_H

/**
 * Tells if overloaded value is odd
 * @param val: int value
 * @return bool: true or false
 */
bool isOdd(int val);

/**
 * Tells if overloaded value is even
 * @param val: int value
 * @return bool: true or false
 */
bool isEven(int val);

/**
 * Tells if overloaded dividend is divisable by the divisor
 * @param dividend: int value to be divided by divisor
 * @param divisor: int divisor
 * @return bool: true or false
 */
bool isDivisable(int dividend, int divisor);

/**
 * Finds the next number that is divisable by divisor from dividend
 * @param dividend: int value to check if divisable by divisor
 * @param divisor: int divisor
 * @return int: Next value that is divisable by divisor, which can also be the dividend itself
 */
int findNextDivisable(int dividend, int divisor);

#endif
