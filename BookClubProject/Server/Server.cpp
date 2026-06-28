// Server.cpp
#include "Server.h"
#include "ClientHandler.h"
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
}

Server::~Server()
{
    stop();
}

bool Server::start(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start on port" << port << ":" << errorString();
        return false;
    }

    qDebug() << "Server started on port" << port;
    return true;
}

void Server::stop()
{
    close();

    for (ClientHandler* client : m_clients.values()) {
        client->deleteLater();
    }
    m_clients.clear();

    qDebug() << "Server stopped";
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New client connected:" << socketDescriptor;

    ClientHandler* handler = new ClientHandler(socketDescriptor, this);

    connect(handler, &ClientHandler::disconnected, this, [this, socketDescriptor]() {
        qDebug() << "Client disconnected:" << socketDescriptor;
        m_clients.remove(socketDescriptor);
    });

    m_clients[socketDescriptor] = handler;
}