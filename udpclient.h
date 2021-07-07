#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTimer>
#include <QFile>
#include <QDataStream>

#include "client.h"
#include "mytypes.h"
#include "incomingdatagram.h"


/**
 * @brief Реализация клиента чата
 *
 * Реализован на udp сокетах;
 * для каждого клиента необходимо задать адрес, к которому будет привязан сокет;
 * также задать адрес получателя.
 *  (адрес вместе с портом)
 */
class UDPClient : public QObject
{
    Q_OBJECT
public:
    explicit UDPClient(QObject *parent = nullptr);
    ~UDPClient();

    void setDatagramSize(uint &d_size);

    bool bindLocal(const QString &ip_addr, const quint16 &port);
    bool connectTo(const QString &ip_addr, const quint16 &port);
    void sendMessage(const QString &message);
    bool sendFile(const QString &file_name);

    void setInterval(const uint &ms);
    uint getMinDatagramSize(void);


signals:
    void newMessage(const Client &sender, const QString &message);
    void messageDelivered(const Client &sender);

private slots:
    void onReadyRead();
    void sendDatagram();

private:

    // udp сокет
    QUdpSocket _socket;

    // размер сообщения в пакете
    uint datagram_size;

    // интервал отправки пакета
    uint interval;

    // размер служебной информации в пакете, например, - 9 байт:
    // первый - флаг, является ли передаваемое сообщение файлом,
    // следующие 4 - общее количество пакетов сообщения,
    // следующие 4 - порядковый номер пакета
    uint metadata_size;

    // размер числа, хранящего количество пакетов, соответствует count_size
    uint byte_count_size;

    // максимально допустимый размер названия файла (байты)
    uint file_name_size;

    // получатель
    Client receiver;

    // таймер, для задания частоты отправки пакетов
    QTimer *tmr;

    // текущее отправляемое сообщение
    QVector<QByteArray> lates_message;

    // содержит пакеты, которые необходимо отправить
    QVector<QByteArray> message_to_send;

    // текущее входящее сообщение
    QHash<count_size, QByteArray> current_incoming_message;


    QByteArray numberToByte(const count_size &number, const uint &count_b);

    QByteArray formDeliveredAnswer(const count_size &count);

    void sendByteData(const QByteArray &data, bool is_file = false);

    QByteArray formFileByteData(const QString &file_name);

    QString processIncomingFile(const QByteArray &datagram);

};

#endif // UDPCLIENT_H
