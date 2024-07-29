#include <iostream>
#include <cmath>
#include <limits>
#include <string>

#include "lib/foo.h"
#include "lib/_quat.h"

using namespace std;

using FQuat = TQuat<float>;

template<typename T>
bool almostEqualRelative(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()) {
    T diff = std::fabs(a - b);
    T largest = std::max(std::fabs(a), std::fabs(b));
    
    return diff <= largest * epsilon;
}

template<typename T>
bool almostEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()) {
    // cout << "epsilon: " << epsilon << ", " << std::fabs(a - b) << endl;
    // return std::fabs(a - b) < epislon;
    return almostEqualRelative(a, b, 1e-6f);
}

bool testBasicMultiplication();
bool testDecimalMultiplication();
bool testNegativeMultiplication();
bool testNegativeDecimalMultiplication();

int main() {
    std::cout << "test.cpp::main()" << std::endl;
    doit();
    int failedTests = 0, testCount = 0;
    failedTests += !testBasicMultiplication();
    testCount++;
    failedTests += !testDecimalMultiplication();
    testCount++;
    failedTests += !testNegativeMultiplication();
    testCount++;
    failedTests += !testNegativeDecimalMultiplication();

    if (failedTests > 0) {
        cout << "Failed " << failedTests << " tests" << endl;
        cout << "Passed: " << testCount << " tests" << endl;
    }
    else {
        cout << "All tests passed: " << testCount << endl;
    }

    return 0;
}

bool testBasicMultiplication() {
    FQuat quatA, quatB, quatC;
    quatA.X = 2.0f;
    quatA.Y = 3.0f;
    quatA.Z = 4.0f;
    quatA.W = 1.0f;

    quatB.X = 6.0f;
    quatB.Y = 7.0f;
    quatB.Z = 8.0f;
    quatB.W = 5.0f;


    quatC = quatA * quatB;

    // cout << "quatC.X: " << quatC.X << endl;
    // cout << "quatC.Y: " << quatC.Y << endl;
    // cout << "quatC.Z: " << quatC.Z << endl;
    // cout << "quatC.W: " << quatC.W << endl;

    bool result = almostEqual(quatC.X, 12.0f) && almostEqual(quatC.Y, 30.0f) && almostEqual(quatC.Z, 24.0f) && almostEqual(quatC.W, -60.0f);
    if (!result) {
        cout << "testBasicMultiplication() failed" << endl;
    }
    return result;
}

bool testDecimalMultiplication() {
    FQuat quatA, quatB, quatC;
    quatA.X = 2.2f;
    quatA.Y = 3.3f;
    quatA.Z = 4.4f;
    quatA.W = 1.1f;

    quatB.X = 6.6f;
    quatB.Y = 7.7f;
    quatB.Z = 8.8f;
    quatB.W = 5.5f;

    quatC = quatA * quatB;

    // cout << "quatC.X: " << quatC.X << endl;
    // cout << "quatC.Y: " << quatC.Y << endl;
    // cout << "quatC.Z: " << quatC.Z << endl;
    // cout << "quatC.W: " << quatC.W << endl;

    
    bool result = almostEqual(quatC.X, 14.52f) && almostEqual(quatC.Y, 36.3f) && almostEqual(quatC.Z, 29.04f) && almostEqual(quatC.W, -72.6f);
    if (!almostEqual(quatC.X, 14.52f)) {
        cout << "quatC.X: " << quatC.X - 14.52f << endl;
    }    
    if (!almostEqual(quatC.Y, 36.3f)) {
        cout << "quatC.Y: " << quatC.Y - 36.3f << endl;
    }
    if (!almostEqual(quatC.Z, 29.04f)) {
        cout << "quatC.Z: " << quatC.Z - 29.04f << endl;
    }
    if (!almostEqual(quatC.W, -72.6f)) {
        cout << "quatC.W: " << quatC.W - -72.6f << endl;
    }


    if (!result) {
        cout << "testDecimalMultiplication() failed" << endl;
    }
    return result;
}

bool testNegativeMultiplication() {
    FQuat quatA, quatB, quatC;
    quatA.X = -22;
    quatA.Y = 33;
    quatA.Z = -44;
    quatA.W = 11;

    quatB.X = 66;
    quatB.Y = -77;
    quatB.Z = 88;
    quatB.W = -55;

    quatC = quatA * quatB;

    // cout << "quatC.X: " << quatC.X << endl;
    // cout << "quatC.Y: " << quatC.Y << endl;
    // cout << "quatC.Z: " << quatC.Z << endl;
    // cout << "quatC.W: " << quatC.W << endl;

    bool result = quatC.X == 1452 && quatC.Y == -3630 && quatC.Z == 2904 && quatC.W == 7260;
    if (!result) {
        cout << "testNegativeMultiplication() failed" << endl;
    }
    return result;
}




bool testNegativeDecimalMultiplication() {
    FQuat quatA, quatB, quatC;
    quatA.X = -22.22f;
    quatA.Y = 33.33f;
    quatA.Z = -44.44f;
    quatA.W = 11.11f;

    quatB.X = 66.66f;
    quatB.Y = -77.77f;
    quatB.Z = 88.88f;
    quatB.W = -55.55f;

    quatC = quatA * quatB;

    // cout << "quatC.X: " << quatC.X << endl;
    // cout << "quatC.Y: " << quatC.Y << endl;
    // cout << "quatC.Z: " << quatC.Z << endl;
    // cout << "quatC.W: " << quatC.W << endl;

    bool result = almostEqual(quatC.X, 1481.1852f) && almostEqual(quatC.Y, -3702.963f) && almostEqual(quatC.Z, 2962.3704f) && almostEqual(quatC.W, 7405.926f);
    if (!result) {
        cout << "testNegativeDecimalMultiplication() failed" << endl;
    }
    if (!almostEqual(quatC.X, 1481.1852f)) {
        cout << "quatC.X: " << quatC.X - 1481.1852f << endl;
    }    
    if (!almostEqual(quatC.Y, -3702.963f)) {
        cout << "quatC.Y: " << quatC.Y - -3702.963f << endl;
    }
    if (!almostEqual(quatC.Z, 2962.3704f)) {
        cout << "quatC.Z: " << quatC.Z - 2962.3704f << endl;
    }
    if (!almostEqual(quatC.W, 7405.926f)) {
        cout << "quatC.W: " << quatC.W - 7405.926f << endl;
    }
    return result;
}