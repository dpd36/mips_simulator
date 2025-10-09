// Author: Dayton Deatherage

// This program demonstrates floating-point overflow 
//   by showing when increments are "lost" due to limited precision.
//   It uses bit manipulation to show the internal representation of floats
//   and checks for overflow in a manner matching IEEE 754 behavior

#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cfloat>

using namespace std;

// Converts float to 32-bit bitset
bitset<32> floatToBits(float floatValue) 
{
    uint32_t floatBits;
    memcpy(&floatBits, &floatValue, sizeof(float));
    return bitset<32>(floatBits);
}

// Prints bits in format: sign exponent fraction
void printFormattedBits(bitset<32> bits) 
{
    cout << bits[31] << " ";
    for (int i = 30; i >= 23; --i) cout << bits[i];
    cout << " ";
    for (int i = 22; i >= 0; --i) cout << bits[i];
    cout << endl;
}

// Finds overflow threshold: where adding increment no longer changes value
float findThreshold(float increment) 
{
    return ldexp(increment, 24);
}

int main(int argCount, char* argValues[]) 
{
    // Checks number of arguments
    if (argCount != 3) 
    {
        cout << "usage: \n\t./fp_overflow_checker loop_bound loop_counter" << endl;
        cout << "\n\tloop_bound is a positive floating-point value" << endl;
        cout << "\tloop_counter is a positive floating-point value\n" << endl;
        return 1;
    }

    // Converts arguments to float
    float loopBoundValue = atof(argValues[1]);
    float loopCounterIncrement = atof(argValues[2]);

    // Prints argumemts' bit representations
    cout << "\nLoop bound:   ";
    printFormattedBits(floatToBits(loopBoundValue));

    cout << "Loop counter: ";
    printFormattedBits(floatToBits(loopCounterIncrement));

    // Prevents increment of 0 from printing possible overflow warning
    if (loopCounterIncrement == 0.0f) 
    {
        cout << "\nThere is no overflow!" << endl;
        return 0;
    }

    // Call to find threshold
    float overflowThreshold = findThreshold(loopCounterIncrement);

    // Prints overflow warning or no overflow
    if (loopBoundValue > overflowThreshold) 
    {
        cout << "\nWarning: Possible overflow!" << endl;
        cout << "Overflow threshold:" << endl;
        cout << "\t" << setprecision(5) << scientific << overflowThreshold << endl;
        cout << "\t";
        printFormattedBits(floatToBits(overflowThreshold));
    } 
    else
        cout << "There is no overflow!" << endl;

    return 0;
}
