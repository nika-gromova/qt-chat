#include "udpclient.h"


UDPClient::UDPClient(QObject *parent) : QObject(parent)
{
    connect(&_socket, &QUdpSocket::readyRead, this, &UDPClient::onReadyRead);
    metadata_size = 9;
    byte_count_size = 4;

    file_name_size = 260;

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
 * @brief Передача сообщения
 * @param message - текст сообщения
 *
 * иходное тескствое сообщение конвертируется в массив байт и отправляется
 *
 */
void UDPClient::sendMessage(const QString &message)
{
    QByteArray ba_message = message.toUtf8();
    sendByteData(ba_message);
}


/**
 * @brief Передача файла
 * @param file_name - название файла (полный путь)
 * @return true, если файл был отправлен, false, если произошла ошибка
 *
 * файл считывается полностью как последовательность байт (ограничение размера
 * файла - 2ГБ);
 * собственно название файла должно быть меньше 260 байт.
 */
bool UDPClient::sendFile(const QString &file_name)
{
    QByteArray file_byte_data = formFileByteData(file_name);
    if (file_byte_data.isEmpty())
    {
        return false;
    }
    sendByteData(file_byte_data, true);
    return true;
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
 * @brief Получение минимального размера пакета, исходя из размера служебной
 * информации
 * @return минимальный размер пакета данных
 */
uint UDPClient::getMinDatagramSize()
{
    return metadata_size + 1;
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
 * если все пакеты дошли, то формируется сообщение из пакетов по порядку;
 * если пришел файл, то из сообщения извлекается название файла, а сам файл
 * записывается в рабочую директорию.
 * также отправляется информация о том, что сообщение было доставлено.
 *
 */
void UDPClient::onReadyRead()
{
    IncomingDatagram datagram;
    QByteArray message;
    QNetworkDatagram n_datagram;

    while (_socket.hasPendingDatagrams())
    {
        n_datagram = _socket.receiveDatagram(_socket.pendingDatagramSize());
        datagram.processDatagram(n_datagram, byte_count_size, metadata_size);

        if (datagram.isDelivered())
        {
            emit messageDelivered(datagram.getSender());
            return;
        }

        current_incoming_message.insert(datagram.getPosition(),
                                        datagram.getData());
    }

    if (current_incoming_message.size() != datagram.getTotalCount())
    {
        return;
    }

    for (auto i = 0; i < current_incoming_message.size(); i++)
    {
        message.append(current_incoming_message[i]);
    }

    if (!datagram.isFile())
    {
        emit newMessage(datagram.getSender(), QString(message));
    }
    else
    {
        emit newMessage(datagram.getSender(), processIncomingFile(message));
    }

    current_incoming_message.clear();
    _socket.writeDatagram(formDeliveredAnswer(datagram.getTotalCount()),
                          datagram.getSender().getAddress(),
                          datagram.getSender().getPort());
}


/**
 * @brief Преобразование числа в массив 4х байт
 * @param number - исходное число
 * @return 4х байтный массив с числом
 */
QByteArray UDPClient::numberToByte(const count_size &number,
                                   const uint &count_b)
{
    QByteArray result, tmp;
    result.resize(count_b);
    result.fill('0');
    tmp.setNum(number, 16);
    result.replace(count_b - tmp.size(), tmp.size(), tmp);
    return result;
}


/**
 * @brief Формирование пакета в случае успешной доставки сообщения
 * @param count - кодичество пакетов доставленного сообщения
 * @return "служебный пакет" -
 * первые 4 байта равны вторым 4м = количеству пакетов
 */
QByteArray UDPClient::formDeliveredAnswer(const count_size &count)
{
    QByteArray answer;
    answer.append('0');
    answer.append(numberToByte(count, byte_count_size));
    answer.append(numberToByte(count, byte_count_size));
    answer.append("/0");
    return answer;
}


/**
 * @brief Передача бинарных данных
 * @param ba_message - данные (массив байт)
 * @param is_file - флаг: true - передаваемые данные файл,
 * иначе - обычное текстовое сообщение
 *
 * исходные данные разделяются на пакеты размером datagram_size, сначала к
 * каждому пакету добавляется флаг файла; далее к каждому пакету в начало
 * добавляется порядковый номер пакета, потом еще общее
 * количество пакетов, на которое разделилось исходное сообщение
 */
void UDPClient::sendByteData(const QByteArray &ba_message, bool is_file)
{
    lates_message.clear();
    count_size ba_size = ba_message.size();
    QByteArray position, total_count;
    count_size count = 0;
    char file_flag = '0';

    if (is_file)
    {
        file_flag = '1';
    }

    for (count_size pos = 0; pos < ba_size; pos += datagram_size, count++)
    {
        QByteArray datagram = ba_message.mid(pos, datagram_size);
        position = numberToByte(count, byte_count_size);
        datagram.prepend(position);
        lates_message.append(datagram);
    }

    total_count = numberToByte(count, byte_count_size);

    for (auto i = 0; i < lates_message.size(); i++)
    {
        lates_message[i].prepend(total_count);
        lates_message[i].prepend(file_flag);
        message_to_send.append(lates_message[i]);
    }
}


/**
 * @brief Представление файла как последовательности байт
 * @param file_name - название файла (полный путь)
 * @return сформироанная последовательность байт
 *
 * формируется короткое имя файла (собственно название);
 * короткое название записывается как часть массива байт данных файла, которое
 * далее будет извлечено на принимающей стороне.
 *
 */
QByteArray UDPClient::formFileByteData(const QString &file_name)
{
    QFile user_file(file_name);
    QByteArray file_data;
    QString short_file_name = file_name.mid(file_name.lastIndexOf("/") + 1);
    QByteArray file_name_b = short_file_name.toUtf8();
    if (file_name_b.size() >= file_name_size)
    {
        return file_data;
    }
    if (user_file.open(QIODevice::ReadOnly))
    {
        file_data = user_file.readAll();
        user_file.close();

        if (file_data.isEmpty())
        {
            return file_data;
        }

        file_name_b.append('\0');
        file_name_b.resize(file_name_size);
        file_data.prepend(file_name_b);
    }
    return file_data;
}


/**
 * @brief Обработка входящего файла
 * @param datagram - последовательность байт, содержащая файл
 * @return сообщение (либо название доставленного файла, либо сообщение
 * об ошибке
 *
 * из начала сообщения извлекается название файла, далее в рабочей директории
 * создается файл с этим название и туда записываются полученные данные
 *
 */
QString UDPClient::processIncomingFile(const QByteArray &datagram)
{
    QString result_message;

    QByteArray file_name_b, result_data;
    QString file_name;
    for (auto i = 0; i < file_name_size; i++)
    {
        file_name_b.append(datagram[i]);
    }
    file_name = QString(file_name_b);
    result_data = datagram.mid(file_name_size);
    QFile new_file(file_name);
    int result = 0;
    if (new_file.open(QIODevice::ReadWrite))
    {
        result = new_file.write(result_data);
        if (result != result_data.size())
        {
            result_message = "Ошибка получения файла";
        }
        else
        {
            result_message = "Вы получили файл: " + file_name;
        }
        new_file.close();
    }
    else
    {
        result_message = "Невозможно получить файл";
    }
    return result_message;
}


/**
 * @brief Отправляет пакет по истечению интервала
 *
 */
void UDPClient::sendDatagram()
{
    if (message_to_send.isEmpty())
    {
        return;
    }

    _socket.writeDatagram(message_to_send.last(),
                          receiver.getAddress(),
                          receiver.getPort());
    message_to_send.removeLast();
}
