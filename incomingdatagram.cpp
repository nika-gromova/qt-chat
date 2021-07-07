#include "incomingdatagram.h"


IncomingDatagram::IncomingDatagram()
{

}

void IncomingDatagram::processDatagram(const QNetworkDatagram &n_datagram,
                                       const count_size &byte_count_size,
                                       const count_size &metadata_size)
{
    bool ok;
    QByteArray datagram = n_datagram.data();
    sender.setAddress(n_datagram.senderAddress());
    sender.setPort(n_datagram.senderPort());

    is_file = (datagram[0] == '1') ? true : false;

    QByteArray count = datagram.mid(1, byte_count_size);
    total_count = count.toUInt(&ok, 16);

    QByteArray pos = datagram.mid(byte_count_size + 1, byte_count_size);
    position = pos.toUInt(&ok, 16);

    byte_data = datagram.mid(metadata_size);
}

bool IncomingDatagram::isFile() const
{
    return is_file;
}

bool IncomingDatagram::isDelivered() const
{
    return (total_count == position);
}

count_size IncomingDatagram::getPosition() const
{
    return position;
}

count_size IncomingDatagram::getTotalCount() const
{
    return total_count;
}

QByteArray IncomingDatagram::getData() const
{
    return byte_data;
}

Client IncomingDatagram::getSender() const
{
    return sender;
}
