#include "utils.h"

#include "classes/library.h"

#include <chrono>
#include <cctype>
#include <functional>
#include <iostream>
#include <limits>

using namespace std;

/**
 * @brief Reads an integer from user input with validation.
 * 
 * Repeatedly prompts the user until valid integer input is received.
 * Clears input buffer on failure to prevent infinite loops.
 * 
 * @param prompt The prompt message to display to the user.
 * @return The valid integer entered by the user.
 */
int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            // Clears all and any leftovers in the input buffer
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid input. Please enter a number." << endl;
        // Clears all and any leftovers in the input buffer
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

/**
 * @brief Reads a positive integer from user input with validation.
 * 
 * Repeatedly prompts the user until a valid positive integer is received.
 * Validates that input contains only digits and is greater than zero.
 * 
 * @param prompt The prompt message to display to the user.
 * @param errorMessage The error message to display on invalid input.
 * @return The valid positive integer entered by the user.
 */
int readPositiveInteger(const string& prompt, const string& errorMessage) {
    // Gets a line of input and checks if it's a positive integer without using stoi or regex.
    while (true) {
        string input;
        cout << prompt;
        getline(cin, input);
        if (input.empty()) {
            cout << errorMessage << endl;
            continue;
        }

        // Check if all characters are digits
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

        // Convert string to integer
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


/**
 * @brief Reads a line of input from the user.
 * 
 * Displays a prompt and reads an entire line of input (including spaces)
 * from standard input.
 * 
 * @param prompt The prompt message to display to the user.
 * @return The input line entered by the user.
 */
string readLine(const string& prompt) {
    // Reads a line of input from the user after displaying a prompt. Returns the input string.
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}


/**
 * @brief Generates a unique random book ID for the library.
 * 
 * Creates a pseudo-random 6-digit book ID (between 100000 and 999999) using
 * the current high-resolution clock time combined with a salt value and hash function.
 * Ensures the generated ID does not collide with existing book IDs in the library.
 * Falls back to incrementing from 1000000 in the unlikely event of repeated collisions.
 * 
 * @param library Reference to the Library object to check for ID collisions.
 * @return A unique book ID that is not already present in the library.
 */
int generateBookId(Library& library) {
    // Generates a random 6-digit book ID that doesn't collide with existing IDs in the library.
    using namespace chrono;

    // Combines current time ticks with a salt and hashes it to produce a pseudo-random ID.
    unsigned long long ticks =
        static_cast<unsigned long long>(high_resolution_clock::now().time_since_epoch().count());
    hash<unsigned long long> hasher;

    // Retry with different salts until we find an unused positive ID.
    for (int salt = 0; salt < 1000; salt++) {
        unsigned long long mixed = ticks + static_cast<unsigned long long>(salt) * 2654435761ULL; // Knuth's multiplicative hash constant
        int candidate = static_cast<int>(hasher(mixed) % 900000ULL) + 100000; // Ensures a 6-digit number between 100000 and 999999
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
