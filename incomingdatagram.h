#ifndef INCOMINGDATAGRAM_H
#define INCOMINGDATAGRAM_H

#include <QByteArray>
#include <QNetworkDatagram>
#include "client.h"
#include "mytypes.h"

class IncomingDatagram
{
public:
    IncomingDatagram();
    void processDatagram(const QNetworkDatagram &n_datagram,
                         const count_size &byte_count_size,
                         const count_size &metadata_size);

    bool isFile(void) const;
    bool isDelivered(void) const;

    count_size getPosition(void) const;
    count_size getTotalCount(void) const;
    QByteArray getData(void) const;
    Client getSender(void) const;


private:
    QByteArray byte_data;
    count_size total_count;
    count_size position;
    Client sender;
    bool is_file;
};

#endif // INCOMINGDATAGRAM_H
