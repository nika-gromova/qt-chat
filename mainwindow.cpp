#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    validator_ipv4(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"))
{
    ui->setupUi(this);

    validator_int = new QIntValidator(1, 10000, this);

    ui->ip_address_sender->setValidator(&validator_ipv4);
    ui->port_sender->setValidator(validator_int);

    ui->ip_address_receiver->setValidator(&validator_ipv4);
    ui->port_receiver->setValidator(validator_int);

    ui->datagram_size->setValidator(validator_int);
    ui->interval->setValidator(validator_int);

    connected_local = false;
    connected_remote = false;

    uint d_size = ui->datagram_size->text().toUInt();
    client.setDatagramSize(d_size);

    connect(&client, UDPClient::newMessage, this, &MainWindow::on_new_message);
    connect(&client, UDPClient::messageDelivered,
            this, &MainWindow::on_message_delivered);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete validator_int;
}


void MainWindow::on_new_message(const Client &sender,
                                const QString &message)
{
    QString new_message = sender.formPrettyAddress() + ": " + message;
    ui->message_list->addItem(new_message);
}

void MainWindow::on_message_delivered(const Client &sender)
{
    QString new_message = "Ваше сообщение доставлено: " +
            sender.formPrettyAddress();
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
 * 10 минимально, так как 9 - размер служебной информации, 1 - минимум
 * для передачи сообщения
 * 8192 максимально, исходня из документации QUdpSocket
 */
bool MainWindow::checkSize(const uint &size)
{
    if (size >= (client.getMinDatagramSize()) && size <= 8192)
        return true;
    return false;
}



void MainWindow::on_save_interval_clicked()
{
    int interval = ui->interval->text().toUInt();
    if  (interval <= 0)
    {
        QMessageBox::warning(this, "Внимание",
                             "Интервал должен быть больше нуля");
        return;
    }
    client.setInterval(interval);
    QMessageBox::information(this, "Ok", "Изменения сохранены");
}

void MainWindow::on_file_btn_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Выбере файл");
    client.sendFile(file_name);
}
