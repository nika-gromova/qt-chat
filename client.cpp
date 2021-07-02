#include "client.h"

Client::Client()
{
    _address = QHostAddress("127.0.0.1");
    _port = 134;
}

Client::Client(const QHostAddress &address, const quint16 &port)
{
    _address = address;
    _port = port;
}

void Client::setAddress(const QHostAddress &address)
{
    _address = address;
}

void Client::setPort(const quint16 &port)
{
    _port = port;
}

QHostAddress Client::getAddress() const
{
    return _address;
}

quint16 Client::getPort() const
{
    return _port;
}
