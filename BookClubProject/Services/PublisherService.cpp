// publisherservice.cpp
#include "PublisherService.h"

PublisherService::PublisherService(BookRepository* repo)
    : bookRepo(repo) {
}


int PublisherService::getBooksPublishedCount(int publisherId) const {
    int count = 0;
    for (Book* book : bookRepo->getAllBooks()) {
        if (book->getPublisherId() == publisherId && book->getIsActive()) {
            count++;
        }
    }
    return count;
}

QVector<Book*> PublisherService::getBooksByPublisher(int publisherId) const {
    QVector<Book*> result;
    for (Book* book : bookRepo->getAllBooks()) {
        if (book->getPublisherId() == publisherId && book->getIsActive()) {
            result.append(book);
        }
    }
    return result;
}