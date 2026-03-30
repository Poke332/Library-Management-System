#include "book.h"

#include <iostream>

using namespace std;

Book::Book()
    : bookId(0), title(""), author(""), totalCopies(0), availableCopies(0) {}

Book::Book(int id, string t, string a, int copies)
    : bookId(id), title(t), author(a), totalCopies(copies), availableCopies(copies) {}

int Book::getBookId() {
    return bookId;
}

string Book::getTitle() {
    return title;
}

string Book::getAuthor() {
    return author;
}

int Book::getTotalCopies() {
    return totalCopies;
}

int Book::getAvailCopies() {
    return availableCopies;
}

void Book::setBookId(int id) {
    bookId = id;
}

void Book::setTitle(string t) {
    title = t;
}

void Book::setAuthor(string a) {
    author = a;
}

void Book::setTotalCopies(int copies) {
    totalCopies = copies;
    if (availableCopies > totalCopies) {
        availableCopies = totalCopies;
    }
}

void Book::setAvailCopies(int copies) {
    availableCopies = copies;
}

void Book::removeCopy() {
    if (availableCopies > 0) {
        availableCopies--;
    }
}

void Book::addCopy() {
    if (availableCopies < totalCopies) {
        availableCopies++;
    }
}

void Book::displayDetails() {
    cout << "Book ID: " << bookId << endl;
    cout << "Book Title: " << title << endl;
    cout << "Book Author: " << author << endl;
    cout << "Copies: " << availableCopies << " Available, " << totalCopies << " Total" << endl;
}