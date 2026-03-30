#include "utils.h"

#include "classes/library.h"

#include <chrono>
#include <cctype>
#include <functional>
#include <iostream>
#include <limits>

using namespace std;

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid input. Please enter a number." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readPositiveInteger(const string& prompt, const string& errorMessage) {
    while (true) {
        string input;
        cout << prompt;
        getline(cin, input);
        if (input.empty()) {
            cout << errorMessage << endl;
            continue;
        }

        bool allDigits = true;
        for (char c : input) {
            if (!isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }
        if (!allDigits) {
            cout << errorMessage << endl;
            continue;
        }

        int value = 0;
        for (char c : input) {
            value = value * 10 + (c - '0');
        }
        if (value <= 0) {
            cout << errorMessage << endl;
            continue;
        }

        return value;
    }
}

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

int generateBookId(Library& library) {
    using namespace chrono;

    unsigned long long ticks =
        static_cast<unsigned long long>(high_resolution_clock::now().time_since_epoch().count());
    hash<unsigned long long> hasher;

    // Retry with different salts until we find an unused positive ID.
    for (int salt = 0; salt < 1000; salt++) {
        unsigned long long mixed = ticks + static_cast<unsigned long long>(salt) * 2654435761ULL;
        int candidate = static_cast<int>(hasher(mixed) % 900000ULL) + 100000;
        if (library.getBookById(candidate) == nullptr) {
            return candidate;
        }
    }

    // Fallback in the unlikely event of repeated collisions.
    int fallback = 1000000;
    while (library.getBookById(fallback) != nullptr) {
        fallback++;
    }
    return fallback;
}
