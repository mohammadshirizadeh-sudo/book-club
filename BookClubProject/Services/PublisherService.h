// publisherservice.h
#ifndef PUBLISHERSERVICE_H
#define PUBLISHERSERVICE_H

#include <QVector>
#include "../Shared/Publisher.h"
#include "../Repositories/BookRepository.h"

class PublisherService {
private:
    BookRepository* bookRepo;

public:
    explicit PublisherService(BookRepository* repo);

    int getBooksPublishedCount(int publisherId) const;
    QVector<Book*> getBooksByPublisher(int publisherId) const;
    BookRepository *getBookRepo() const;
};

#endif // PUBLISHERSERVICE_H