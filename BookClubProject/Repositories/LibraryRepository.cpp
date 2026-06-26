// libraryrepository.cpp
#include "LibraryRepository.h"
#include <QDebug>

LibraryRepository::LibraryRepository() {
    // TODO: Load from file in Phase 3
}

LibraryRepository::~LibraryRepository() {
    // Clean up all libraries to prevent memory leak
    qDeleteAll(librariesByUserId);
}

bool LibraryRepository::addLibrary(Library* library) {
    if (!library) {
        qWarning() << "Library is null!";
        return false;
    }

    int userId = library->getUserId();

    // Check if library already exists
    if (librariesByUserId.contains(userId)) {
        qWarning() << "Library for user" << userId << "already exists!";
        return false;
    }

    librariesByUserId[userId] = library;
    qDebug() << "Library added for user:" << userId;
    return true;
}

Library* LibraryRepository::findByUserId(int userId) const {
    return librariesByUserId.value(userId, nullptr);
}

QVector<Library*> LibraryRepository::getAllLibraries() const {
    return librariesByUserId.values().toVector();
}

bool LibraryRepository::updateLibrary(Library* library) {
    if (!library) {
        qWarning() << "Library is null!";
        return false;
    }

    int userId = library->getUserId();
    if (!librariesByUserId.contains(userId)) {
        qWarning() << "Library for user" << userId << "not found!";
        return false;
    }

    librariesByUserId[userId] = library;
    qDebug() << "Library updated for user:" << userId;
    return true;
}

bool LibraryRepository::deleteLibrary(int userId) {
    Library* library = librariesByUserId.value(userId, nullptr);
    if (!library) {
        qWarning() << "Library for user" << userId << "not found!";
        return false;
    }

    librariesByUserId.remove(userId);
    delete library;
    qDebug() << "Library deleted for user:" << userId;
    return true;
}

bool LibraryRepository::exists(int userId) const {
    return librariesByUserId.contains(userId);
}