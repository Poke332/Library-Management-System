#include "save_handler.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace {

/// Reads entire file contents into a string.
/// Implementation: Opens the file and streams its contents into an ostringstream buffer,
/// then returns the buffer as a string. Returns empty string on open failure.
/// @param path File path to read
/// @return File contents as string, or empty string if file cannot be opened
string readFile(const string& path) {
    ifstream in(path);
    if (!in.is_open()) {
        return "";
    }
    // Use rdbuf() to efficiently read entire file into buffer
    ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

/// Escapes special characters in a string for JSON serialization.
/// Implementation: Iterates through each character and prepends a backslash to any
/// backslash or quote character. This prevents JSON parsing errors when strings contain
/// these special characters.
/// @param input String to escape
/// @return Escaped string with backslashes and quotes properly escaped
string escapeJson(const string& input) {
    string out;
    for (char c : input) {
        // Add escape backslash before quotes and existing backslashes
        if (c == '\\' || c == '"') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

/// Unescapes JSON-encoded string, reversing escapeJson.
/// Implementation: Maintains an 'escaped' flag to track if the previous character was a
/// backslash. When escaped=true, the current character is added without question and
/// the flag is reset. This handles both escaped quotes and escaped backslashes.
/// @param input Escaped JSON string
/// @return Unescaped string
string unescapeJson(const string& input) {
    string out;
    bool escaped = false; // Flag indicating if previous character was a backslash
    for (char c : input) {
        if (escaped) {
            out.push_back(c); // Add the escaped character as-is
            escaped = false; // Reset flag
        } else if (c == '\\') {
            escaped = true; // Set flag for next iteration
        } else {
            out.push_back(c); // Add normal characters
        }
    }
    return out;
}

/// Advances position past whitespace characters.
/// Implementation: Increments the position counter while the current character is
/// space, newline, tab, or carriage return. Stops at first non-whitespace character
/// or end of string.
/// @param s String to process
/// @param i Starting position
/// @return Position of first non-whitespace character, or end of string if none found
size_t skipWhitespace(const string& s, size_t i) {
    // Skip over space, newline, tab, and carriage return characters
    while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r')) {
        i++;
    }
    return i;
}

/// Extracts the content of a JSON array associated with a given key.
/// Implementation: Locates the key in JSON, finds the array opening bracket,
/// then uses depth tracking to handle nested arrays. Ignores brackets inside
/// strings by tracking inString and escaped states. Returns content without
/// the outer brackets.
/// @param json JSON string to parse
/// @param key Array key to locate
/// @return Content of the array without brackets, or empty string if not found
string extractArrayContent(const string& json, const string& key) {
    string marker = "\"" + key + "\"";
    size_t keyPos = json.find(marker);
    if (keyPos == string::npos) return "";

    // Find the colon separating key from value
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == string::npos) return "";

    size_t start = skipWhitespace(json, colonPos + 1);
    if (start >= json.size() || json[start] != '[') return "";

    // Track nesting depth to handle nested arrays correctly
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = start; i < json.size(); i++) {
        char c = json[i];
        if (inString) {
            // Handle escape sequences inside strings
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') inString = false;
            continue;
        }
        if (c == '"') inString = true;
        else if (c == '[') depth++;
        else if (c == ']') {
            depth--;
            // Return array content when outer bracket closes (depth == 0)
            if (depth == 0) return json.substr(start + 1, i - (start + 1));
        }
    }
    return "";
}

/// Reads the next JSON object from array content.
/// Implementation: Advances to the next opening brace, then uses depth tracking
/// to extract the complete object including all nested braces. Handles strings
/// and escapes to avoid being fooled by braces inside string values.
/// @param arrayContent Content of a JSON array
/// @param pos Position to start reading from (updated to position after object)
/// @param outObject Output parameter containing the extracted object
/// @return True if object was successfully extracted, false if end of array reached
bool readNextObject(const string& arrayContent, size_t& pos, string& outObject) {
    pos = skipWhitespace(arrayContent, pos);
    // Advance to next opening brace
    while (pos < arrayContent.size() && arrayContent[pos] != '{') pos++;
    if (pos >= arrayContent.size()) return false;

    size_t start = pos;
    int depth = 0; // Track brace nesting depth
    bool inString = false;
    bool escaped = false;
    for (; pos < arrayContent.size(); pos++) {
        char c = arrayContent[pos];
        if (inString) {
            // Handle escape sequences inside strings
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') inString = false;
            continue;
        }
        if (c == '"') inString = true;
        else if (c == '{') depth++;
        else if (c == '}') {
            depth--;
            // Complete object found when depth returns to 0
            if (depth == 0) {
                outObject = arrayContent.substr(start, pos - start + 1);
                pos++; // Move past the closing brace for next call
                return true;
            }
        }
    }
    return false;
}

/// Extracts an integer value from a JSON object by key.
/// Implementation: Locates the key, finds the colon, then parses the value as a
/// decimal integer. Handles negative numbers and validates that digits follow.
/// @param obj JSON object string
/// @param key Key to search for
/// @param out Output parameter for extracted integer
/// @return True if integer was successfully extracted, false otherwise
bool extractInt(const string& obj, const string& key, int& out) {
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) return false;

    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) return false;

    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size()) return false;

    // Check for negative sign
    bool negative = false;
    if (obj[i] == '-') { negative = true; i++; }
    // Ensure at least one digit exists
    if (i >= obj.size() || obj[i] < '0' || obj[i] > '9') return false;

    // Build integer by accumulating digit values
    int value = 0;
    while (i < obj.size() && obj[i] >= '0' && obj[i] <= '9') {
        value = value * 10 + (obj[i] - '0');
        i++;
    }
    out = negative ? -value : value;
    return true;
}

/// Extracts a string value from a JSON object by key.
/// Implementation: Locates the key, finds the colon, then reads characters until
/// a closing quote is found. Handles escape sequences by accumulating them into
/// a raw string, then unescapes the result.
/// @param obj JSON object string
/// @param key Key to search for
/// @param out Output parameter for extracted string
/// @return True if string was successfully extracted, false otherwise
bool extractString(const string& obj, const string& key, string& out) {
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) return false;

    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) return false;

    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size() || obj[i] != '"') return false;

    i++; // Move past opening quote
    string raw; // Accumulate raw characters (including escape sequences)
    bool escaped = false;
    for (; i < obj.size(); i++) {
        char c = obj[i];
        if (escaped) { 
            raw.push_back(c); // Add escaped character
            escaped = false; 
        }
        else if (c == '\\') escaped = true; // Mark next character as escaped
        else if (c == '"') { 
            out = unescapeJson(raw); // Found closing quote, unescape and return
            return true; 
        }
        else raw.push_back(c);
    }
    return false;
}

/// Extracts an integer array from a JSON object by key.
/// Uses a two-pass approach to safely allocate memory and avoid memory leaks.
/// Pass 1 counts valid integers; Pass 2 allocates exact size and fills array.
/// @param obj JSON object string
/// @param key Array key to search for
/// @param outCount Output parameter for array size
/// @return Dynamically allocated integer array (caller must delete[]), or nullptr if not found
int* extractIntArray(const string& obj, const string& key, int& outCount) {
    outCount = 0;
 
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) return nullptr;
 
    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) return nullptr;
 
    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size() || obj[i] != '[') return nullptr;
 
    // --- Pass 1: count valid integers ---
    size_t scanPos = i + 1;
    int count = 0;
    while (scanPos < obj.size()) {
        scanPos = skipWhitespace(obj, scanPos);
        if (scanPos >= obj.size() || obj[scanPos] == ']') break;
        if (obj[scanPos] == '-') scanPos++;
        if (scanPos >= obj.size() || obj[scanPos] < '0' || obj[scanPos] > '9') break;
        while (scanPos < obj.size() && obj[scanPos] >= '0' && obj[scanPos] <= '9') scanPos++;
        count++;
        scanPos = skipWhitespace(obj, scanPos);
        if (scanPos < obj.size() && obj[scanPos] == ',') scanPos++;
    }
 
    if (count == 0) return nullptr;
 
    // --- Pass 2: allocate once, then fill ---
    int* values = new int[count];
    int idx = 0;
    i = i + 1; // move past '['
    while (i < obj.size() && idx < count) {
        i = skipWhitespace(obj, i);
        if (i >= obj.size() || obj[i] == ']') break;
 
        bool negative = false;
        if (obj[i] == '-') { negative = true; i++; }
        if (i >= obj.size() || obj[i] < '0' || obj[i] > '9') break;
 
        int value = 0;
        while (i < obj.size() && obj[i] >= '0' && obj[i] <= '9') {
            value = value * 10 + (obj[i] - '0');
            i++;
        }
        values[idx++] = negative ? -value : value;
 
        i = skipWhitespace(obj, i);
        if (i < obj.size() && obj[i] == ',') i++;
    }
 
    outCount = idx;
    return values;
}

} // namespace

namespace SaveHandler {

/// Saves all library data (books, students, metadata) to JSON files.
/// Implementation: Creates output directory, then writes three JSON files:
/// - books.json: Array of book objects with metadata
/// - students.json: Array of student objects with borrowed book IDs
/// - library.json: Summary counts (bookCount, studentCount)
/// Tracks written count to ensure correct JSON comma placement (no trailing commas).
/// @param library Reference to Library object to save
/// @param directory Directory path where JSON files will be written
/// @return True if all files saved successfully, false if any file operation fails
bool saveAll(Library& library, const string& directory) {
    filesystem::create_directories(directory);

    // FIX BUG #2: Trailing comma in JSON output
    // Collect valid pointers first, then write with correct comma placement.

    // --- Save books ---
    ofstream booksFile(directory + "/books.json");
    if (!booksFile.is_open()) return false;

    // Pre-count valid books to avoid trailing commas in JSON
    int bookCount = library.getBookCount();
    int validBookCount = 0;
    for (int i = 0; i < bookCount; i++) {
        if (library.getBookAt(i) != nullptr) validBookCount++;
    }

    booksFile << "{\n \"books\": [\n";
    // Write each book, tracking count to place commas correctly (no trailing comma)
    int written = 0;
    for (int i = 0; i < bookCount; i++) {
        Book* book = library.getBookAt(i);
        if (book == nullptr) continue;

            // Write book object: bookId, title, author, totalCopies, availableCopies
        booksFile << "    {\"bookId\": " << book->getBookId()
                  << ", \"title\": \"" << escapeJson(book->getTitle())
                  << "\", \"author\": \"" << escapeJson(book->getAuthor())
                  << "\", \"totalCopies\": " << book->getTotalCopies()
                  << ", \"availableCopies\": " << book->getAvailCopies() << "}";

        written++;
        // Add comma only if not the last item to prevent trailing comma in JSON
        if (written < validBookCount) booksFile << ",";
        booksFile << "\n";
    }
    booksFile << "  ]\n}\n";

    // --- Save students ---
    ofstream studentsFile(directory + "/students.json");
    if (!studentsFile.is_open()) return false;

    int studentCount = library.getStudentCount();
    int validStudentCount = 0;
    for (int i = 0; i < studentCount; i++) {
        if (library.getStudentAt(i) != nullptr) validStudentCount++;
    }

    studentsFile << "{\n \"students\": [\n";
    written = 0;
    for (int i = 0; i < studentCount; i++) {
        Student* student = library.getStudentAt(i);
        if (student == nullptr) continue;

        studentsFile << "    {\"studentId\": " << student->getStudentId()
                     << ", \"name\": \"" << escapeJson(student->getName())
                     << "\", \"borrowedBookIds\": [";

        for (int j = 0; j < student->getBorrowCount(); j++) {
            studentsFile << student->getBorrowedBookIdAt(j);
            if (j < student->getBorrowCount() - 1) studentsFile << ", ";
        }
        studentsFile << "]}";

        written++;
        if (written < validStudentCount) studentsFile << ",";
        studentsFile << "\n";
    }
    studentsFile << "  ]\n}\n";

    // --- Save library metadata ---
    ofstream libraryFile(directory + "/library.json");
    if (!libraryFile.is_open()) return false;

    // Write summary statistics to library.json
    libraryFile << "{\n";
    libraryFile << "  \"bookCount\": " << library.getBookCount() << ",\n";
    libraryFile << "  \"studentCount\": " << library.getStudentCount() << "\n";
    libraryFile << "}\n";

    return true;
}

/// Loads all library data from JSON files in the specified directory.
/// Implementation: Reads books.json and students.json, parsing JSON arrays
/// and extracting objects. Validates data ranges (e.g., availableCopies clamped
/// to [0, totalCopies]). Returns true even if files are missing (graceful loading).
/// @param library Reference to Library object to populate
/// @param directory Directory path containing JSON files to load
/// @return True if loading completed (even if some files are missing), false on parse errors
bool loadAll(Library& library, const string& directory) {
    // Read both JSON files (returns empty string if file not found)
    string booksJson    = readFile(directory + "/books.json");
    string studentsJson = readFile(directory + "/students.json");

    // Gracefully return if both files are missing
    if (booksJson.empty() && studentsJson.empty()) return true;

    // --- Load books ---
    string booksArray = extractArrayContent(booksJson, "books");
    size_t bookPos = 0;
    string obj;
    while (readNextObject(booksArray, bookPos, obj)) {
        int bookId = 0, totalCopies = 0, availableCopies = 0;
        string title, author;

        // Extract required fields; skip object if any fail
        if (!extractInt(obj, "bookId", bookId)) continue;
        if (!extractInt(obj, "totalCopies", totalCopies)) continue;
        if (!extractInt(obj, "availableCopies", availableCopies)) availableCopies = totalCopies;

        // Validate and clamp availableCopies to [0, totalCopies] range
        if (availableCopies < 0) availableCopies = 0;
        if (availableCopies > totalCopies) availableCopies = totalCopies;

        // Optional fields; defaults used if not found
        extractString(obj, "title", title);
        extractString(obj, "author", author);

        // Create book and add to library
        Book book(bookId, title, author, totalCopies);
        book.setAvailCopies(availableCopies);
        library.addBook(book);
    }

    // --- Load students ---
    string studentsArray = extractArrayContent(studentsJson, "students");
    size_t studentPos = 0;
    while (readNextObject(studentsArray, studentPos, obj)) {
        int studentId = 0;
        string name;

        // Extract required field; skip object if fails
        if (!extractInt(obj, "studentId", studentId)) continue;
        // Optional name field
        extractString(obj, "name", name);

        // Create and add student to library
        Student student(studentId, name);
        library.addStudent(student);

        // Retrieve pointer to newly added student for borrowed books
        Student* loadedStudent = library.getStudentById(studentId);
        if (loadedStudent == nullptr) continue;

        // Extract borrowed book IDs array and re-associate books
        int borrowCount = 0;
        int* borrowedIds = extractIntArray(obj, "borrowedBookIds", borrowCount);
        for (int i = 0; i < borrowCount; i++) {
            int bookId = borrowedIds[i];
            Book* book = library.getBookById(bookId);
            if (book == nullptr) continue; // Skip if book not found
            if (loadedStudent->hasBorrowedBook(bookId)) continue; // Skip duplicates
            // Re-establish the borrow relationship
            loadedStudent->borrowBook(bookId);
        }
        delete[] borrowedIds; // Free dynamically allocated array
    }

    return true;
}

} // namespace SaveHandler