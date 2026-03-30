#pragma once

#include <string>

class Student {
private:
    int studentId;
    std::string name;
    int* borrowedBookIds;
    int borrowCount;

public:
    Student();
    Student(int id, std::string studentName);
    Student(const Student& other);
    Student& operator=(const Student& other);
    ~Student();

    int getStudentId();
    std::string getName();
    bool hasBorrowedBook(int bookId);
    int getBorrowCount();
    int getBorrowedBookIdAt(int index);

    void borrowBook(int bookId);
    void returnBook(int bookId);
    void displayBorrowed();
};
