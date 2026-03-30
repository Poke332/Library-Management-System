#include "save_handler.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace {

string readFile(const string& path) {
    ifstream in(path);
    if (!in.is_open()) {
        return "";
    }
    ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

string escapeJson(const string& input) {
    string out;
    for (char c : input) {
        if (c == '\\' || c == '"') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

string unescapeJson(const string& input) {
    string out;
    bool escaped = false;
    for (char c : input) {
        if (escaped) {
            out.push_back(c);
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

size_t skipWhitespace(const string& s, size_t i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r')) {
        i++;
    }
    return i;
}

string extractArrayContent(const string& json, const string& key) {
    string marker = "\"" + key + "\"";
    size_t keyPos = json.find(marker);
    if (keyPos == string::npos) {
        return "";
    }

    size_t colonPos = json.find(':', keyPos);
    if (colonPos == string::npos) {
        return "";
    }
    size_t start = skipWhitespace(json, colonPos + 1);
    if (start >= json.size() || json[start] != '[') {
        return "";
    }

    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = start; i < json.size(); i++) {
        char c = json[i];
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }

        if (c == '"') {
            inString = true;
        } else if (c == '[') {
            depth++;
        } else if (c == ']') {
            depth--;
            if (depth == 0) {
                return json.substr(start + 1, i - (start + 1));
            }
        }
    }
    return "";
}

bool readNextObject(const string& arrayContent, size_t& pos, string& outObject) {
    pos = skipWhitespace(arrayContent, pos);
    while (pos < arrayContent.size() && arrayContent[pos] != '{') {
        pos++;
    }
    if (pos >= arrayContent.size()) {
        return false;
    }

    size_t start = pos;
    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (; pos < arrayContent.size(); pos++) {
        char c = arrayContent[pos];
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }

        if (c == '"') {
            inString = true;
        } else if (c == '{') {
            depth++;
        } else if (c == '}') {
            depth--;
            if (depth == 0) {
                outObject = arrayContent.substr(start, pos - start + 1);
                pos++;
                return true;
            }
        }
    }

    return false;
}

bool extractInt(const string& obj, const string& key, int& out) {
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) {
        return false;
    }
    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) {
        return false;
    }
    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size()) {
        return false;
    }

    bool negative = false;
    if (obj[i] == '-') {
        negative = true;
        i++;
    }
    if (i >= obj.size() || obj[i] < '0' || obj[i] > '9') {
        return false;
    }

    int value = 0;
    while (i < obj.size() && obj[i] >= '0' && obj[i] <= '9') {
        value = value * 10 + (obj[i] - '0');
        i++;
    }
    out = negative ? -value : value;
    return true;
}

bool extractString(const string& obj, const string& key, string& out) {
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) {
        return false;
    }
    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) {
        return false;
    }
    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size() || obj[i] != '"') {
        return false;
    }
    i++;

    string raw;
    bool escaped = false;
    for (; i < obj.size(); i++) {
        char c = obj[i];
        if (escaped) {
            raw.push_back(c);
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            out = unescapeJson(raw);
            return true;
        } else {
            raw.push_back(c);
        }
    }
    return false;
}

int* extractIntArray(const string& obj, const string& key, int& outCount) {
    outCount = 0;
    int* values = nullptr;
    string marker = "\"" + key + "\"";
    size_t keyPos = obj.find(marker);
    if (keyPos == string::npos) {
        return values;
    }
    size_t colonPos = obj.find(':', keyPos);
    if (colonPos == string::npos) {
        return values;
    }
    size_t i = skipWhitespace(obj, colonPos + 1);
    if (i >= obj.size() || obj[i] != '[') {
        return values;
    }
    i++;

    while (i < obj.size()) {
        i = skipWhitespace(obj, i);
        if (i >= obj.size()) {
            break;
        }
        if (obj[i] == ']') {
            break;
        }

        bool negative = false;
        if (obj[i] == '-') {
            negative = true;
            i++;
        }
        if (i >= obj.size() || obj[i] < '0' || obj[i] > '9') {
            break;
        }
        int value = 0;
        while (i < obj.size() && obj[i] >= '0' && obj[i] <= '9') {
            value = value * 10 + (obj[i] - '0');
            i++;
        }
        int* newValues = new int[outCount + 1];
        for (int idx = 0; idx < outCount; idx++) {
            newValues[idx] = values[idx];
        }
        newValues[outCount] = negative ? -value : value;
        delete[] values;
        values = newValues;
        outCount++;

        i = skipWhitespace(obj, i);
        if (i < obj.size() && obj[i] == ',') {
            i++;
        }
    }

    return values;
}

} // namespace

namespace SaveHandler {

bool saveAll(Library& library, const string& directory) {
    filesystem::create_directories(directory);

    ofstream booksFile(directory + "/books.json");
    if (!booksFile.is_open()) {
        return false;
    }
    booksFile << "{\n  \"books\": [\n";
    for (int i = 0; i < library.getBookCount(); i++) {
        Book* book = library.getBookAt(i);
        if (book == nullptr) {
            continue;
        }
        booksFile << "    {\"bookId\": " << book->getBookId()
                  << ", \"title\": \"" << escapeJson(book->getTitle())
                  << "\", \"author\": \"" << escapeJson(book->getAuthor())
                  << "\", \"totalCopies\": " << book->getTotalCopies()
                  << ", \"availableCopies\": " << book->getAvailCopies() << "}";
        if (i < library.getBookCount() - 1) {
            booksFile << ",";
        }
        booksFile << "\n";
    }
    booksFile << "  ]\n}\n";

    ofstream studentsFile(directory + "/students.json");
    if (!studentsFile.is_open()) {
        return false;
    }
    studentsFile << "{\n  \"students\": [\n";
    for (int i = 0; i < library.getStudentCount(); i++) {
        Student* student = library.getStudentAt(i);
        if (student == nullptr) {
            continue;
        }

        studentsFile << "    {\"studentId\": " << student->getStudentId()
                     << ", \"name\": \"" << escapeJson(student->getName())
                     << "\", \"borrowedBookIds\": [";
        for (int j = 0; j < student->getBorrowCount(); j++) {
            studentsFile << student->getBorrowedBookIdAt(j);
            if (j < student->getBorrowCount() - 1) {
                studentsFile << ", ";
            }
        }
        studentsFile << "]}";
        if (i < library.getStudentCount() - 1) {
            studentsFile << ",";
        }
        studentsFile << "\n";
    }
    studentsFile << "  ]\n}\n";

    ofstream libraryFile(directory + "/library.json");
    if (!libraryFile.is_open()) {
        return false;
    }
    libraryFile << "{\n";
    libraryFile << "  \"bookCount\": " << library.getBookCount() << ",\n";
    libraryFile << "  \"studentCount\": " << library.getStudentCount() << "\n";
    libraryFile << "}\n";

    return true;
}

bool loadAll(Library& library, const string& directory) {
    string booksJson = readFile(directory + "/books.json");
    string studentsJson = readFile(directory + "/students.json");

    if (booksJson.empty() && studentsJson.empty()) {
        return true;
    }

    string booksArray = extractArrayContent(booksJson, "books");
    size_t bookPos = 0;
    string obj;
    while (readNextObject(booksArray, bookPos, obj)) {
        int bookId = 0;
        int totalCopies = 0;
        int availableCopies = 0;
        string title;
        string author;

        if (!extractInt(obj, "bookId", bookId)) {
            continue;
        }
        if (!extractInt(obj, "totalCopies", totalCopies)) {
            continue;
        }
        if (!extractInt(obj, "availableCopies", availableCopies)) {
            availableCopies = totalCopies;
        }
        extractString(obj, "title", title);
        extractString(obj, "author", author);

        Book book(bookId, title, author, totalCopies);
        book.setAvailCopies(availableCopies);
        library.addBook(book);
    }

    string studentsArray = extractArrayContent(studentsJson, "students");
    size_t studentPos = 0;
    while (readNextObject(studentsArray, studentPos, obj)) {
        int studentId = 0;
        string name;

        if (!extractInt(obj, "studentId", studentId)) {
            continue;
        }
        extractString(obj, "name", name);

        Student student(studentId, name);
        library.addStudent(student);
        Student* loadedStudent = library.getStudentById(studentId);
        if (loadedStudent == nullptr) {
            continue;
        }

        int borrowCount = 0;
        int* borrowedIds = extractIntArray(obj, "borrowedBookIds", borrowCount);
        for (int i = 0; i < borrowCount; i++) {
            int bookId = borrowedIds[i];
            Book* book = library.getBookById(bookId);
            if (book == nullptr) {
                continue;
            }
            if (loadedStudent->hasBorrowedBook(bookId)) {
                continue;
            }
            loadedStudent->borrowBook(bookId);
        }
        delete[] borrowedIds;
    }

    return true;
}

} // namespace SaveHandler
