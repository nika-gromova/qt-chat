#include "udpclient.h"


UDPClient::UDPClient(QObject *parent) : QObject(parent)
{
    connect(&_socket, &QUdpSocket::readyRead, this, &UDPClient::onReadyRead);
    metadata_size = 8;

    tmr = new QTimer(this);
    connect(tmr, &QTimer::timeout, this, &UDPClient::sendDatagram);
    tmr->start(100);
}


UDPClient::~UDPClient()
{
    delete tmr;
}


/**
 * @brief Установление размера передаваемого пакета
 * @param d_size - размер пакета
 *
 * размер передаваемой информации в пакете + размер служебной информации -
 * передаваемый d_size
 */
void UDPClient::setDatagramSize(uint &d_size)
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
bool UDPClient::bindLocal(const QString &ip_addr, const quint16 &port)
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
        receiver.setAddress(QHostAddress(ip_addr));
        receiver.setPort(port);
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
    lates_message.clear();
    QByteArray ba_message = message.toUtf8();
    quint32 ba_size = ba_message.size();
    QByteArray position, total_count;
    quint32 count = 0;

    for (quint32 pos = 0; pos < ba_size; pos += datagram_size, count++)
    {
        QByteArray datagram = ba_message.mid(pos, datagram_size);
        position = numberTo4byte(count);
        datagram.prepend(position);
        lates_message.append(datagram);
    }

    total_count = numberTo4byte(count);

    for (auto i = 0; i < lates_message.size(); i++)
    {
        lates_message[i].prepend(total_count);
        message_to_send.append(lates_message[i]);
    }
}


/**
 * @brief Установка интервала отправки пакетов
 * @param ms - временной интервал (в миллисекундах)
 *
 */
void UDPClient::setInterval(const uint &ms)
{
    interval = ms;
    tmr->setInterval(interval);
}


/**
 * @brief Слот срабатывает, если есть доступные для чтения пакеты
 *
 * принимаются пакеты, пока есть доступные,
 * из каждого извлекаестя порядковый номер и общее количество
 * далее пакеты помещаются в ассоциативный массив, ключом которого является
 * порядковый номер, а значением - содержимое пакета
 *
 * далее если дошли не все пакеты, то ждем, пока дойдут все,
 * если все пакеты дошли, то формируется сообщение из пакетов по порядку
 *
 */
void UDPClient::onReadyRead()
{
    Client sender;
    QByteArray datagram, count, position;
    QNetworkDatagram n_datagram;
    quint32 total_count = 0, current_position = 0;

    while (_socket.hasPendingDatagrams())
    {
        n_datagram = _socket.receiveDatagram(_socket.pendingDatagramSize());
        datagram = n_datagram.data();
        sender.setAddress(n_datagram.senderAddress());
        sender.setPort(n_datagram.senderPort());

        count = datagram.mid(0, 4);
        total_count = count.toUInt();

        position = datagram.mid(4, 4);
        current_position = position.toUInt();

        if (total_count == current_position)
        {
            emit messageDelivered(sender);
            return;
        }

        datagram = datagram.mid(8);
        current_incoming_message.insert(current_position, datagram);
    }

    if (current_incoming_message.size() != total_count)
    {
        return;
    }

    datagram.clear();
    for (auto i = 0; i < total_count; i++)
    {
        datagram.append(current_incoming_message[i]);
    }
    emit newMessage(sender, datagram);
    current_incoming_message.clear();
    _socket.writeDatagram(formDeliveredAnswer(total_count),
                          sender.getAddress(),
                          sender.getPort());
}


/**
 * @brief Преобразование числа в массив 4х байт
 * @param number - исходное число
 * @return 4х байтный массив с числом
 */
QByteArray UDPClient::numberTo4byte(const quint32 &number)
{
    QByteArray result, tmp;
    result.resize(4);
    result.fill('0');
    tmp.setNum(number);
    result.replace(4 - tmp.size(), tmp.size(), tmp);
    return result;
}


/**
 * @brief Формирование пакета в случае успешной доставки сообщения
 * @param count - кодичество пакетов доставленного сообщения
 * @return "служебный пакет" -
 * первые 4 байта равны вторым 4м = количеству пакетов
 */
QByteArray UDPClient::formDeliveredAnswer(const quint32 &count)
{
    QByteArray answer;
    answer.append(numberTo4byte(count));
    answer.append(numberTo4byte(count));
    answer.append("/0");
    return answer;
}


/**
 * @brief Отправляет пакет по истечению интервала
 *
 */
void UDPClient::sendDatagram()
{
    if (message_to_send.isEmpty())
        return;

    _socket.writeDatagram(message_to_send.last(),
                          receiver.getAddress(),
                          receiver.getPort());
    message_to_send.removeLast();
}
