#include <iostream>
#include "lib/foo.h"
#include <string>
#include "lib/_quat.h"

using namespace std;

using FQuat = TQuat<float>;

int main() {
    std::cout << "Hello, World!!" << std::endl;
    doit();
    string a = "fooa";
    string b = "foo";
    b += "a";
    cout << "a == b: " << (a == b) << endl;

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

    cout << "quatC.X: " << quatC.X << endl;
    cout << "quatC.Y: " << quatC.Y << endl;
    cout << "quatC.Z: " << quatC.Z << endl;
    cout << "quatC.W: " << quatC.W << endl;

    return 0;
}
