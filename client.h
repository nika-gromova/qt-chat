#ifndef CLIENT_H
#define CLIENT_H

#include <QHostAddress>

class Client
{
public:
    Client();
    Client(const QHostAddress &address, const quint16 &port);

    void setAddress(const QHostAddress &address);
    void setPort(const quint16 &port);

    QHostAddress getAddress(void) const;
    quint16 getPort(void) const;


private:
    // адрес ipv4
    QHostAddress _address;

    // порт
    quint16 _port;
};

#endif // CLIENT_H
