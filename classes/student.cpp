#include "student.h"

#include <iostream>

using namespace std;

Student::Student()
    : studentId(0), name(""), borrowedBookIds(nullptr), borrowCount(0) {}

Student::Student(int id, string studentName)
    : studentId(id), name(studentName), borrowedBookIds(nullptr), borrowCount(0) {}

Student::Student(const Student& other)
    : studentId(other.studentId), name(other.name), borrowedBookIds(nullptr), borrowCount(other.borrowCount) {
    if (borrowCount > 0) {
        borrowedBookIds = new int[borrowCount];
        for (int i = 0; i < borrowCount; i++) {
            borrowedBookIds[i] = other.borrowedBookIds[i];
        }
    }
}

Student& Student::operator=(const Student& other) {
    if (this == &other) {
        return *this;
    }

    delete[] borrowedBookIds;
    studentId = other.studentId;
    name = other.name;
    borrowCount = other.borrowCount;
    borrowedBookIds = nullptr;

    if (borrowCount > 0) {
        borrowedBookIds = new int[borrowCount];
        for (int i = 0; i < borrowCount; i++) {
            borrowedBookIds[i] = other.borrowedBookIds[i];
        }
    }
    return *this;
}

Student::~Student() {
    delete[] borrowedBookIds;
}

int Student::getStudentId() {
    return studentId;
}

string Student::getName() {
    return name;
}

bool Student::hasBorrowedBook(int bookId) {
    for (int i = 0; i < borrowCount; i++) {
        if (borrowedBookIds[i] == bookId) {
            return true;
        }
    }
    return false;
}

int Student::getBorrowCount() {
    return borrowCount;
}

int Student::getBorrowedBookIdAt(int index) {
    if (index < 0 || index >= borrowCount) {
        return -1;
    }
    return borrowedBookIds[index];
}

void Student::borrowBook(int bookId) {
    int* newBorrowed = new int[borrowCount + 1];
    for (int i = 0; i < borrowCount; i++) {
        newBorrowed[i] = borrowedBookIds[i];
    }
    newBorrowed[borrowCount] = bookId;

    delete[] borrowedBookIds;
    borrowedBookIds = newBorrowed;
    borrowCount++;
}

void Student::returnBook(int bookId) {
    int index = -1;
    for (int i = 0; i < borrowCount; i++) {
        if (borrowedBookIds[i] == bookId) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "Book ID " << bookId << " not found in borrowed list." << endl;
        return;
    }

    int* newBorrowed = nullptr;
    if (borrowCount - 1 > 0) {
        newBorrowed = new int[borrowCount - 1];
        for (int i = 0, j = 0; i < borrowCount; i++) {
            if (i != index) {
                newBorrowed[j++] = borrowedBookIds[i];
            }
        }
    }

    delete[] borrowedBookIds;
    borrowedBookIds = newBorrowed;
    borrowCount--;
}

void Student::displayBorrowed() {
    cout << "Student ID: " << studentId << endl;
    cout << "Name: " << name << endl;
    cout << "Borrowed Books: ";

    if (borrowCount == 0) {
        cout << "None";
    } else {
        for (int i = 0; i < borrowCount; i++) {
            cout << borrowedBookIds[i];
            if (i < borrowCount - 1) {
                cout << ", ";
            }
        }
    }
    cout << endl;
}