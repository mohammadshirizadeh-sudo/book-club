// publisherservice.h
#ifndef PUBLISHERSERVICE_H
#define PUBLISHERSERVICE_H

#include <QVector>
#include "Publisher.h"
#include "BookReposity.h"

class PublisherService {
private:
    BookRepository* bookRepo;

public:
    explicit PublisherService(BookRepository* repo);

    int getBooksPublishedCount(int publisherId) const;
    QVector<Book*> getBooksByPublisher(int publisherId) const;
};

#endif // PUBLISHERSERVICE_H