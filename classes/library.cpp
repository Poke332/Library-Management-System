#include "library.h"

#include <iostream>

using namespace std;

Library::Library()
    : books(nullptr), students(nullptr), bookCount(0), studentCount(0) {}

Library::~Library() {
    delete[] books;
    delete[] students;
}

Student* Library::findStudentById(int studentId) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].getStudentId() == studentId) {
            return &students[i];
        }
    }
    return nullptr;
}

Book* Library::findBookById(int bookId) {
    for (int i = 0; i < bookCount; i++) {
        if (books[i].getBookId() == bookId) {
            return &books[i];
        }
    }
    return nullptr;
}

void Library::addBook(Book& b) {
    Book* newBooks = new Book[bookCount + 1];
    for (int i = 0; i < bookCount; i++) {
        newBooks[i] = books[i];
    }
    newBooks[bookCount] = b;

    delete[] books;
    books = newBooks;
    bookCount++;
}

void Library::addStudent(Student& s) {
    Student* newStudents = new Student[studentCount + 1];
    for (int i = 0; i < studentCount; i++) {
        newStudents[i] = students[i];
    }
    newStudents[studentCount] = s;

    delete[] students;
    students = newStudents;
    studentCount++;
}

void Library::borrowBook(int studentId, int bookId) {
    Student* student = findStudentById(studentId);
    Book* book = findBookById(bookId);

    if (student == nullptr) {
        cout << "Student not found." << endl;
        return;
    }
    if (book == nullptr) {
        cout << "Book not found." << endl;
        return;
    }
    if (book->getAvailCopies() <= 0) {
        cout << "No available copies for this book." << endl;
        return;
    }

    student->borrowBook(bookId);
    book->removeCopy();
    cout << "Book borrowed successfully." << endl;
}

void Library::returnBook(int studentId, int bookId) {
    Student* student = findStudentById(studentId);
    Book* book = findBookById(bookId);

    if (student == nullptr) {
        cout << "Student not found." << endl;
        return;
    }
    if (book == nullptr) {
        cout << "Book not found." << endl;
        return;
    }
    if (!student->hasBorrowedBook(bookId)) {
        cout << "Student did not borrow this book." << endl;
        return;
    }

    student->returnBook(bookId);
    book->addCopy();
    cout << "Book returned successfully." << endl;
}

int Library::getBookCount() {
    return bookCount;
}

int Library::getStudentCount() {
    return studentCount;
}

Book* Library::getBookAt(int index) {
    if (index < 0 || index >= bookCount) {
        return nullptr;
    }
    return &books[index];
}

Student* Library::getStudentAt(int index) {
    if (index < 0 || index >= studentCount) {
        return nullptr;
    }
    return &students[index];
}

Student* Library::getStudentById(int studentId) {
    return findStudentById(studentId);
}

Book* Library::getBookById(int bookId) {
    return findBookById(bookId);
}

Book* Library::getBookByTitle(const string& title) {
    for (int i = 0; i < bookCount; i++) {
        if (books[i].getTitle() == title) {
            return &books[i];
        }
    }
    return nullptr;
}