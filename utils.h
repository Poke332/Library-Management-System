#pragma once

#include <string>

class Library;

int readInt(const std::string& prompt);
int readPositiveInteger(
    const std::string& prompt,
    const std::string& errorMessage = "Invalid input. Enter a positive integer (no decimals, > 0).");
std::string readLine(const std::string& prompt);
int generateBookId(Library& library);
