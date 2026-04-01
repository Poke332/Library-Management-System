#include "menu.h"
#include "save_handler.h"
#include "utils.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

static void printDivider(int width = 64, char fill = '=') {
    cout << setfill(fill) << setw(width) << "" << setfill(' ') << endl;
}

static void printField(const string& label, const string& value) {
    cout << " " << left << setw(18) << (label + ":") << value << endl;
}

static void printField(const string& label, int value) {
    cout << " " << left << setw(18) << (label + ":") << value << endl;
}

static string fitCell(const string& text, size_t width) {
    if (text.size() <= width) return text;
    if (width <= 3) return text.substr(0, width);
    return text.substr(0, width - 3) + "...";
}

static void printMenu() {
    cout << endl;
    printDivider();
    cout << " Library Management Menu" << endl;
    printDivider();
    cout << " " << left << setw(42) << "1) Add a new book"
         << "2) Add a new student" << endl;
    cout << " " << left << setw(42) << "3) Borrow a book"
         << "4) Return a book" << endl;
    cout << " " << left << setw(42) << "5) Display all books"
         << "6) Display all students" << endl;
    cout << " " << left << setw(42) << "7) Display borrowed books + borrowers"
         << "8) Search book by ID or title" << endl;
    cout << " " << left << setw(42) << "9) Update book info"
         << "10) Exit" << endl;
    printDivider();
}

static void waitForEnter() {
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

static void displayAllBooks(Library& library) {
    if (library.getBookCount() == 0) {
        cout << "No books in library." << endl;
        return;
    }

    const int wId = 10, wTitle = 24, wAuthor = 20, wAvail = 10, wTotal = 10;

    cout << endl;
    printDivider(80, '=');
    cout << "| " << left << setw(wId)    << "Book ID"
         << "| " << setw(wTitle)  << "Title"
         << "| " << setw(wAuthor) << "Author"
         << "| " << setw(wAvail)  << "Available"
         << "| " << setw(wTotal)  << "Total"
         << "|" << endl;
    printDivider(80, '-');

    for (int i = 0; i < library.getBookCount(); i++) {
        Book* book = library.getBookAt(i);
        if (book == nullptr) continue;
        cout << "| " << left << setw(wId)    << book->getBookId()
             << "| " << setw(wTitle)  << fitCell(book->getTitle(), wTitle)
             << "| " << setw(wAuthor) << fitCell(book->getAuthor(), wAuthor)
             << "| " << setw(wAvail)  << book->getAvailCopies()
             << "| " << setw(wTotal)  << book->getTotalCopies()
             << "|" << endl;
    }
    printDivider(80, '=');
}

static void displayAllStudents(Library& library) {
    if (library.getStudentCount() == 0) {
        cout << "No students in library." << endl;
        return;
    }

    const int wId = 12, wName = 20, wBorrowed = 44;

    cout << endl;
    printDivider(86, '=');
    cout << "| " << left << setw(wId)       << "Student ID"
         << "| " << setw(wName)     << "Name"
         << "| " << setw(wBorrowed) << "Borrowed Books"
         << "|" << endl;
    printDivider(86, '-');

    for (int i = 0; i < library.getStudentCount(); i++) {
        Student* student = library.getStudentAt(i);
        if (student == nullptr) continue;

        string borrowedText;
        if (student->getBorrowCount() == 0) {
            borrowedText = "None";
        } else {
            for (int j = 0; j < student->getBorrowCount(); j++) {
                int borrowedBookId = student->getBorrowedBookIdAt(j);
                Book* borrowedBook = library.getBookById(borrowedBookId);
                if (borrowedBook != nullptr) {
                    borrowedText += borrowedBook->getTitle() + "(" + to_string(borrowedBook->getBookId()) + ")";
                } else {
                    borrowedText += "UnknownBook(" + to_string(borrowedBookId) + ")";
                }
                if (j < student->getBorrowCount() - 1) borrowedText += ", ";
            }
        }

        cout << "| " << left << setw(wId)       << student->getStudentId()
             << "| " << setw(wName)     << fitCell(student->getName(), wName)
             << "| " << setw(wBorrowed) << fitCell(borrowedText, wBorrowed)
             << "|" << endl;
    }
    printDivider(86, '=');
}

static void displayBorrowedBooksWithBorrowers(Library& library) {
    int totalBooks    = library.getBookCount();
    int totalStudents = library.getStudentCount();
    bool anyBorrowedBook = false;

    const int wBook = 34, wBorrowers = 40;

    cout << endl;
    printDivider(82, '=');
    cout << "| " << left << setw(wBook)      << "Book"
         << "| " << setw(wBorrowers) << "Borrowed By"
         << "|" << endl;
    printDivider(82, '-');

    for (int i = 0; i < totalBooks; i++) {
        Book* book = library.getBookAt(i);
        if (book == nullptr) continue;

        bool bookIsBorrowed = false;
        for (int j = 0; j < totalStudents; j++) {
            Student* student = library.getStudentAt(j);
            if (student != nullptr && student->hasBorrowedBook(book->getBookId())) {
                bookIsBorrowed = true;
                break;
            }
        }
        if (!bookIsBorrowed) continue;

        anyBorrowedBook = true;
        string borrowersText;
        bool first = true;
        for (int j = 0; j < totalStudents; j++) {
            Student* student = library.getStudentAt(j);
            if (student != nullptr && student->hasBorrowedBook(book->getBookId())) {
                if (!first) borrowersText += ", ";
                borrowersText += student->getName() + "(" + to_string(student->getStudentId()) + ")";
                first = false;
            }
        }

        cout << "| " << left << setw(wBook)
             << fitCell(book->getTitle() + "(" + to_string(book->getBookId()) + ")", wBook)
             << "| " << setw(wBorrowers) << fitCell(borrowersText, wBorrowers)
             << "|" << endl;
    }

    if (!anyBorrowedBook) {
        cout << "No books are currently borrowed by students." << endl;
    }
    printDivider(82, '=');
}

static void printBookDetails(Book* book) {
    if (book == nullptr) {
        cout << "Book not found." << endl;
        return;
    }
    printDivider(64, '-');
    printField("Book ID",          book->getBookId());
    printField("Book Title",       book->getTitle());
    printField("Book Author",      book->getAuthor());
    printField("Available Copies", book->getAvailCopies());
    printField("Total Copies",     book->getTotalCopies());
    printDivider(64, '-');
}

static void searchBook(Library& library) {
    cout << "Search by: 1) ID 2) Title" << endl;
    int mode = readInt("Choose search mode: ");

    if (mode == 1) {
        int id = readPositiveInteger("Enter book ID: ", "Invalid ID. Enter a positive integer.");
        Book* book = library.getBookById(id);
        printBookDetails(book);
    } else if (mode == 2) {
        string titlePart = readLine("Enter title keyword: ");
        bool foundAny = false;
        for (int i = 0; i < library.getBookCount(); i++) {
            Book* book = library.getBookAt(i);
            if (book == nullptr) continue;
            if (book->getTitle().find(titlePart) != string::npos) {
                printBookDetails(book);
                foundAny = true;
            }
        }
        if (!foundAny) {
            cout << "No books found with title containing \"" << titlePart << "\"." << endl;
        }
    } else {
        cout << "Invalid search mode." << endl;
    }
}

static void updateBookData(Library& library) {
    int targetId = readPositiveInteger("Enter book ID to update: ", "Invalid ID. Enter a positive integer.");
    Book* book = library.getBookById(targetId);
    if (book == nullptr) {
        cout << "Book not found." << endl;
        return;
    }

    while (true) {
        cout << endl;
        printDivider();
        cout << " Update Book: " << book->getTitle() << " (" << book->getBookId() << ")" << endl;
        printDivider();
        cout << " 1) Title" << endl;
        cout << " 2) Author" << endl;
        cout << " 3) Total Copies" << endl;
        cout << " 4) Available Copies" << endl;
        cout << " 5) Exit update menu" << endl;
        printDivider();

        int updateChoice = readInt("Choose field to update: ");

        if (updateChoice == 1) {
            string newTitle = readLine("Enter new title: ");
            book->setTitle(newTitle);
            SaveHandler::saveAll(library);
            cout << "Book title updated successfully." << endl;

        } else if (updateChoice == 2) {
            string newAuthor = readLine("Enter new author: ");
            book->setAuthor(newAuthor);
            SaveHandler::saveAll(library);
            cout << "Book author updated successfully." << endl;

        } else if (updateChoice == 3) {
            int newTotal = readPositiveInteger(
                "Enter new total copies: ",
                "Invalid total copies. Enter a positive integer.");

            int oldTotal     = book->getTotalCopies();
            int oldAvailable = book->getAvailCopies();
            int borrowedCount = oldTotal - oldAvailable;

            // FIX BUG #4: Reducing totalCopies below borrowed count
            // Old code only increased availCopies when newTotal > oldTotal,
            // leaving availCopies > newTotal when reducing — an impossible state.
            // Fix: recalculate available copies in both directions, clamping
            // to ensure: 0 <= availCopies <= newTotal and availCopies = newTotal - borrowedCount.
            if (newTotal < borrowedCount) {
                cout << "Cannot reduce total copies below the number currently borrowed ("
                     << borrowedCount << ")." << endl;
                continue;
            }

            book->setTotalCopies(newTotal);
            book->setAvailCopies(newTotal - borrowedCount);
            SaveHandler::saveAll(library);
            cout << "Book total copies updated successfully." << endl;

        } else if (updateChoice == 4) {
            int newAvailable = readPositiveInteger(
                "Enter new available copies: ",
                "Invalid available copies. Enter a positive integer.");

            if (newAvailable > book->getTotalCopies()) {
                cout << "Available copies cannot exceed total copies ("
                     << book->getTotalCopies() << ")." << endl;
                continue;
            }
            book->setAvailCopies(newAvailable);
            SaveHandler::saveAll(library);
            cout << "Book available copies updated successfully." << endl;

        } else if (updateChoice == 5) {
            break;
        } else {
            cout << "Invalid update choice. Please pick 1-5." << endl;
        }
    }
}

void runMenu(Library& library) {
    while (true) {
        printMenu();
        int choice = readInt("Enter choice: ");
        bool shouldExit = false;

        if (choice == 1) {
            string title  = readLine("Title: ");
            string author = readLine("Author: ");
            int copies    = readPositiveInteger("Total copies: ");
            int id        = generateBookId(library);
            Book book(id, title, author, copies);
            library.addBook(book);
            SaveHandler::saveAll(library);
            cout << "Book added successfully with auto-generated ID: " << id << endl;

        } else if (choice == 2) {
            int id = readPositiveInteger("Student ID: ", "Invalid ID. Enter a positive integer.");

            // FIX BUG #5: Duplicate student ID check
            // Old code added the student unconditionally. If the same ID was entered
            // twice, both would exist internally, causing getStudentById() to always
            // return the first one and silently ignore the second.
            if (library.getStudentById(id) != nullptr) {
                cout << "A student with ID " << id << " already exists." << endl;
            } else {
                string name = readLine("Student name: ");
                Student student(id, name);
                library.addStudent(student);
                SaveHandler::saveAll(library);
                cout << "Student added successfully." << endl;
            }

        } else if (choice == 3) {
            int studentId = readPositiveInteger("Student ID: ", "Invalid ID. Enter a positive integer.");
            int bookId    = readPositiveInteger("Book ID: ",    "Invalid ID. Enter a positive integer.");
            library.borrowBook(studentId, bookId);
            SaveHandler::saveAll(library);

        } else if (choice == 4) {
            int studentId = readPositiveInteger("Student ID: ", "Invalid ID. Enter a positive integer.");
            int bookId    = readPositiveInteger("Book ID: ",    "Invalid ID. Enter a positive integer.");
            library.returnBook(studentId, bookId);
            SaveHandler::saveAll(library);

        } else if (choice == 5) {
            displayAllBooks(library);

        } else if (choice == 6) {
            displayAllStudents(library);

        } else if (choice == 7) {
            displayBorrowedBooksWithBorrowers(library);

        } else if (choice == 8) {
            searchBook(library);

        } else if (choice == 9) {
            updateBookData(library);

        } else if (choice == 10) {
            SaveHandler::saveAll(library);
            cout << "Exiting..." << endl;
            shouldExit = true;

        } else {
            cout << "Invalid choice. Please pick 1-10." << endl;
        }

        if (shouldExit) break;
        waitForEnter();
    }
}