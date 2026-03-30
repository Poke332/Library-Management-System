#pragma once

#include <string>

class Book {
private:
    int bookId;
    std::string title;
    std::string author;
    int totalCopies;
    int availableCopies;

public:
    Book();
    Book(int id, std::string t, std::string a, int copies);

    int getBookId();
    std::string getTitle();
    std::string getAuthor();
    int getTotalCopies();
    int getAvailCopies();

    void setBookId(int id);
    void setTitle(std::string t);
    void setAuthor(std::string a);
    void setTotalCopies(int copies);
    void setAvailCopies(int copies);

    void removeCopy();
    void addCopy();
    void displayDetails();
};
