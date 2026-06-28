
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QMap>

class ClientHandler;

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool start(quint16 port);
    void stop();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMap<qintptr, ClientHandler*> m_clients;
};

#endif // SERVER_H