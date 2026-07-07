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
    QMap<int, Library*> librariesByUserId;
    mutable QMutex m_mutex;
    void addToCache(Library* library);
    void removeFromCache(int userId);
    void clearCache();

public:
    LibraryRepository(QObject* parent);
    ~LibraryRepository();

    // ===== CRUD Operations =====
    bool addLibrary(Library* library);

    Library* findByUserId(int userId) const;
    QVector<Library*> getAllLibraries() const;
    bool updateLibrary(Library* library);
    bool deleteLibrary(int userId);
    bool exists(int userId) const;




    bool loadAllFromDatabase();
    bool saveToDatabase(Library* library);
    bool deleteFromDatabase(int userId);
    bool saveShelves(int userId, const QVector<Shelf>& shelves);
    bool loadShelves(Library* library);
};

#endif // LIBRARYREPOSITORY_H