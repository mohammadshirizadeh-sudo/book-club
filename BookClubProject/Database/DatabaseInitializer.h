// DatabaseInitializer.h
#ifndef DATABASEINITIALIZER_H
#define DATABASEINITIALIZER_H

#include <QObject>
#include <QString>

class DatabaseInitializer : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseInitializer(QObject *parent = nullptr);
    ~DatabaseInitializer();

    bool initialize(const QString& databasePath);
    bool createTables();
    bool dropTables();

private:
    bool createUserTable();
    bool createPublisherInfoTable();
    bool createAdminInfoTable();
    bool createBookTable();
    bool createReviewTable();
    bool createPurchaseTable();
    bool createPurchaseItemTable();
    bool createCartTable();
    bool createCartItemTable();
    bool createNotificationTable();
    bool createLibraryTable();
    bool createShelfTable();
    bool createShelfBookTable();
    bool createLibraryOwnedBookTable();

    bool createLibrarySavedBookTable();




    QString m_databasePath;

public:
    bool insertDefaultBooks();
};

#endif // DATABASEINITIALIZER_H