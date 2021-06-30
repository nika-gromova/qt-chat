#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    validator_ipv4(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"))
{
    ui->setupUi(this);

    validator_port = new QIntValidator(1, 65535, this);
    validator_size = new QIntValidator(9, 8192, this);

    ui->ip_address_sender->setValidator(&validator_ipv4);
    ui->port_sender->setValidator(validator_port);

    ui->ip_address_receiver->setValidator(&validator_ipv4);
    ui->port_receiver->setValidator(validator_port);

    ui->datagram_size->setValidator(validator_size);

    connected_local = false;
    connected_remote = false;

    uint d_size = ui->datagram_size->text().toUInt();
    client.setDatagramSize(d_size);

    connect(&client, UDPClient::newMessage, this, &MainWindow::on_new_message);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete validator_port;
    delete validator_size;
}


void MainWindow::on_new_message(const QHostAddress &sender_addr,
                                const quint16 &sender_port,
                                const QByteArray &ba_message)
{
    QString sender_info = sender_addr.toString() + "::" + \
            QString::number(sender_port);
    QString new_message = sender_info + ": " + QString(ba_message);
    ui->message_list->addItem(new_message);
}


void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        on_send_btn_clicked();
    }
}


void MainWindow::on_save_btn_clicked()
{
    QString ip_addr = ui->ip_address_sender->text();
    quint16 port = ui->port_sender->text().toInt();
    if (!checkPort(port))
    {
        QMessageBox::warning(this, "Внимание", "Некорректный порт");
        return;
    }

    bool connected = client.bindLocal(ip_addr, port);
    if (! connected)
    {
        QMessageBox::warning(this, "Внимание", "Не удалось сохранить");
    }
    else
    {
        QMessageBox::information(this, "Ok", "Удалось сохранить");
        connected_local = true;
    }
}


void MainWindow::on_connect_btn_clicked()
{
    QString ip_addr = ui->ip_address_receiver->text();
    quint16 port = ui->port_receiver->text().toInt();

    if (!checkPort(port))
    {
        QMessageBox::warning(this, "Внимание", "Некорректный порт");
        return;
    }

    bool connected = client.connectTo(ip_addr, port);
    if (! connected)
    {
        QMessageBox::warning(this, "Внимание", "Не удалось подключиться");
    }
    else
    {
        QMessageBox::information(this, "Ok", "Удалось подключиться");
        connected_remote = true;
    }
}


void MainWindow::on_send_btn_clicked()
{
    if (connected_local && connected_remote)
    {
        QString message = ui->message->text();
        client.sendMessage(message);
        ui->message->clear();
        ui->message_list->addItem("Вы: " + message);
    }
    else
    {
        QMessageBox::warning(this, "Внимание",
                             "Задайте параметры отправителя и получателя");
    }
}


void MainWindow::on_save_d_size_clicked()
{
    uint d_size = ui->datagram_size->text().toUInt();
    if (!checkSize(d_size))
    {
        QMessageBox::warning(this, "Внимание",
                             "Размер пакета должен быть от 9 до 8192 байт");
        return;
    }
    client.setDatagramSize(d_size);
    QMessageBox::information(this, "Ok", "Изменения сохранены");
}


bool MainWindow::checkPort(const quint16 &port)
{
    if (port >= 1 && port <= 65535)
        return true;
    return false;
}


/**
 * @brief проверка устанавливаемого размера пакета (в байтах)
 * @param size - размер пакета
 * @return true, если число подходит, иначе - false
 *
 * 9 минимально, так как 8 - размер служебной информации, 1 - минимум
 * для передачи сообщения
 * 8192 максимально, исходня из документации QUdpSocket
 */
bool MainWindow::checkSize(const uint &size)
{
    if (size >= 9 && size <= 8192)
        return true;
    return false;
}


