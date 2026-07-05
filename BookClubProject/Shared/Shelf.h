// shelf.h
#ifndef SHELF_H
#define SHELF_H

#include <QString>
#include <QVector>

class Shelf {
private:
    int shelfId;
    QString name;
    QVector<int> bookIds;  // IDs of books in this shelf

public:
    // ===== Constructors =====
    Shelf();
    Shelf(int shelfId, const QString& name);

    // ===== Getters =====
    int getShelfId() const { return shelfId; }
    QString getName() const { return name; }
    QVector<int> getBookIds() const { return bookIds; }
    int getBookCount() const { return bookIds.size(); }

    // ===== Setters =====
    void setShelfId(int id) { shelfId = id; }
    void setName(const QString& name) { this->name = name; }
    void setBookIds(const QVector<int>& ids) { bookIds = ids; }

    // ===== Book Management =====

    bool addBook(int bookId);

    bool removeBook(int bookId);

    bool contains(int bookId) const;

    void clear();

    bool isEmpty() const { return bookIds.isEmpty(); }
};

#endif // SHELF_H