// libraryrepository.h
#ifndef LIBRARYREPOSITORY_H
#define LIBRARYREPOSITORY_H

#include <QMap>
#include <QVector>
#include <QObject>
#include "../Shared/Library.h"

#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"

class LibraryRepository : public QObject {
    Q_OBJECT
private:
    QMap<int, QSharedPointer<Library>> librariesByUserId;
    mutable QMutex m_mutex;
    void addToCache(QSharedPointer<Library> library);
    void removeFromCache(int userId);
    void clearCache();

public:
    LibraryRepository(QObject* parent);
    ~LibraryRepository();

    // ===== CRUD Operations =====
    bool addLibrary(QSharedPointer<Library> library);

    QSharedPointer<Library> findByUserId(int userId) const;
    QVector<QSharedPointer<Library>> getAllLibraries() const;
    bool updateLibrary(QSharedPointer<Library> library);
    bool deleteLibrary(int userId);
    bool exists(int userId) const;




    bool loadAllFromDatabase();
    bool saveToDatabase(QSharedPointer<Library> library);
    bool deleteFromDatabase(int userId);
    bool saveShelves(int userId, const QVector<Shelf>& shelves);
    bool loadShelves(QSharedPointer<Library> library);

    bool saveOwnedBooks(int libraryId, const QVector<int>& ownedBooks);
    bool loadOwnedBooks(QSharedPointer<Library> library, int libraryId);

    // ===== Saved Books =====
    bool saveSavedBooks(int libraryId, const QVector<int>& savedBooks);
    bool loadSavedBooks(QSharedPointer<Library> library, int libraryId);
    int getLibraryDbId(int userId) const;
};

#endif // LIBRARYREPOSITORY_H