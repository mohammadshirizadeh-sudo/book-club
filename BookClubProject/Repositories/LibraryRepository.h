// libraryrepository.h
#ifndef LIBRARYREPOSITORY_H
#define LIBRARYREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Library.h"

/**
 * @brief Repository for managing Library objects in memory
 * Handles CRUD operations for libraries
 */
class LibraryRepository {
private:
    QMap<int, Library*> librariesByUserId;  // Fast lookup by user ID

public:
    LibraryRepository();
    ~LibraryRepository();

    // ===== CRUD Operations =====

    /**
     * @brief Add a new library to repository
     * @param library Library to add
     * @return true if added successfully
     */
    bool addLibrary(Library* library);

    /**
     * @brief Find library by user ID
     * @param userId User ID
     * @return Library pointer or nullptr if not found
     */
    Library* findByUserId(int userId) const;

    /**
     * @brief Get all libraries
     * @return Vector of all libraries
     */
    QVector<Library*> getAllLibraries() const;

    /**
     * @brief Update an existing library
     * @param library Updated library
     * @return true if update was successful
     */
    bool updateLibrary(Library* library);

    /**
     * @brief Delete a library
     * @param userId User ID of library to delete
     * @return true if deleted successfully
     */
    bool deleteLibrary(int userId);

    /**
     * @brief Check if user has a library
     * @param userId User ID
     * @return true if library exists
     */
    bool exists(int userId) const;
};

#endif // LIBRARYREPOSITORY_H