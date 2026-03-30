#pragma once

#include "book.h"
#include "student.h"

class Library {
private:
    Book* books;
    Student* students;
    int bookCount;
    int studentCount;

    Student* findStudentById(int studentId);
    Book* findBookById(int bookId);

public:
    Library();
    ~Library();

    void addBook(Book& b);
    void addStudent(Student& s);
    void borrowBook(int studentId, int bookId);
    void returnBook(int studentId, int bookId);
    
    int getBookCount();
    int getStudentCount();
    Book* getBookAt(int index);
    Student* getStudentAt(int index);
    Student* getStudentById(int studentId);
    Book* getBookById(int bookId);
    Book* getBookByTitle(const std::string& title);
};
