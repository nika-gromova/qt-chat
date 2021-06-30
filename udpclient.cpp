#include "udpclient.h"


UDPClient::UDPClient(QObject *parent) : QObject(parent)
{
    connect(&_socket, &QUdpSocket::readyRead, this, &UDPClient::onReadyRead);
    metadata_size = 8;
}


/**
 * @brief Установление размера передаваемого пакета
 * @param d_size - размер пакета
 *
 * размер передаваемой информации в пакете + размер служебной информации -
 * передаваемый d_size
 */
void UDPClient::set_datagram_size(uint &d_size)
{
    if (d_size > 0)
    {
        datagram_size = d_size - metadata_size;
    }
}


/**
 * @brief Привязка сокета к адресу
 * @param ip_addr - адрес ipv4
 * @param port - порт
 * @return true, если связывание прошло успешно, иначе - false
 */
bool UDPClient::bind_local(const QString &ip_addr, const quint16 &port)
{
    bool connected = false;
    if (_socket.state() != _socket.BoundState)
    {
        connected = _socket.bind(QHostAddress(ip_addr), port);
    }
    return connected;
}


/**
 * @brief Сохранение адреса получателя
 * @param ip_addr - адрес ipv4
 * @param port - порт
 * @return true, если входные данные корректные, иначе - false
 */
bool UDPClient::connectTo(const QString &ip_addr, const quint16 &port)
{
    bool connected = false;
    if (port != 0)
    {
        receiver_ip_address = QHostAddress(ip_addr);
        receiver_port = port;
        connected = true;
    }
    return connected;
}


/**
 * @brief Отправление сообщения
 * @param message - текст сообщения
 *
 * исходное сообщение разделяется на пакеты размером datagram_size,
 * далее к каждому пакету в начало добавляется порядковый номер пакета,
 * потом еще общее количество пакетов, на которое разделилось исходное сообщение
 *
 */
void UDPClient::sendMessage(const QString &message)
{
    current_message.clear();
    QByteArray ba_message = message.toUtf8();
    quint32 ba_size = ba_message.size();
    QByteArray position, total_count;
    quint32 count = 0;

    for (quint32 pos = 0; pos < ba_size; pos += datagram_size, count++)
    {
        QByteArray datagram = ba_message.mid(pos, datagram_size);
        position = number_to_4byte(count);
        datagram.prepend(position);
        current_message.append(datagram);
    }

    total_count = number_to_4byte(count);

    for (auto i = 0; i < current_message.size(); i++)
    {
        current_message[i].prepend(total_count);
        _socket.writeDatagram(current_message[i],
                              receiver_ip_address,
                              receiver_port);
    }

}


/**
 * @brief Слот срабатывает, если есть доступные для чтения пакеты
 *
 * принимаются пакеты, пока есть доступные,
 * из каждого извлекаестя порядковый номер и общее количество
 * далее пакеты помещаются в ассоциативный массив, ключом которого является
 * порядковый номер, а значением - содержимое пакета
 *
 * далее если дошли не все пакеты, то выдается ошибка,
 * если все пакеты дошли, то формируется сообщение из пакетов по порядку
 *
 */
void UDPClient::onReadyRead()
{
    QHostAddress sender_addr;
    quint16 sender_port;
    QByteArray datagram, count, position;
    QNetworkDatagram n_datagram;
    qint64 d_size = 0;
    qint32 total_count = 0, current_position = 0;
    QHash<qint32, QByteArray> current_message;

    while (_socket.hasPendingDatagrams())
    {
        d_size = _socket.pendingDatagramSize();
        datagram.resize(d_size);

        n_datagram = _socket.receiveDatagram(d_size);
        datagram = n_datagram.data();
        sender_addr = n_datagram.senderAddress();
        sender_port = n_datagram.senderPort();

        count = datagram.mid(0, 4);
        total_count = count.toUInt();

        position = datagram.mid(4, 4);
        current_position = position.toUInt();

        datagram = datagram.mid(8);
        current_message.insert(current_position, datagram);
    }

    if (current_message.size() != total_count)
    {
        emit newMessage(sender_addr, sender_port, QByteArray("Error"));
        return;
    }

    datagram.clear();
    for (auto i = 0; i < total_count; i++)
    {
        datagram.append(current_message[i]);
    }
    emit newMessage(sender_addr, sender_port, datagram);

}


/**
 * @brief Преобразование числа в массив 4х байт
 * @param number - исходное число
 * @return 4х байтный массив с числом
 */
QByteArray UDPClient::number_to_4byte(const quint32 &number)
{
    QByteArray result, tmp;
    result.resize(4);
    result.fill('0');
    tmp.setNum(number);
    result.replace(4 - tmp.size(), tmp.size(), tmp);
    return result;
}
