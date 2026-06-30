// UserFactory.h
#ifndef USERFACTORY_H
#define USERFACTORY_H

#include "User.h"
#include "Admin.h"
#include "Publisher.h"


namespace UserFactory {

inline User* createUser(int id, const QString& username, const QString& email,
                        const QString& password, UserRole role,
                        const QString& publisherName = "") {
    switch(role) {
    case UserRole::Admin:
        return new Admin(id, username, email, password);

    case UserRole::Publisher:

        return new Publisher(id, username, email, password, publisherName);

    default:
        return new User(id, username, email, password);
    }
}

}

#endif // USERFACTORY_H