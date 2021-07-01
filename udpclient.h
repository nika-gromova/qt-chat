#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTimer>

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
    void setInterval(const uint &ms);

signals:
    void newMessage(const QHostAddress &sender_addr, const quint16 &sender_port,
                    const QByteArray &ba_message);

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

    // размер служебной информации в пакете - 8 байт:
    // первые 4 - общее количество пакетов сообщения,
    // вторые 4 - порядковый номер пакета
    uint metadata_size;

    // адрес получателя
    QHostAddress receiver_ip_address;

    // порт получателя
    quint16 receiver_port;

    // таймер, для задания частоты отправки пакетов
    QTimer *tmr;

    // текущее отправляемое сообщение
    QVector<QByteArray> lates_message;

    // содержит пакеты, которые необходимо отправить
    QVector<QByteArray> message_to_send;

    // текущее входящее сообщение
    QHash<qint32, QByteArray> current_incoming_message;


    QByteArray numberTo4byte(const quint32 &number);
};

#endif // UDPCLIENT_H
