#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QMessageBox>
#include <QKeyEvent>

#include "udpclient.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_new_message(const QHostAddress &sender_addr,
                        const quint16 &sender_port,
                        const QByteArray &ba_message);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void on_connect_btn_clicked();

    void on_save_btn_clicked();

    void on_send_btn_clicked();

    void on_save_d_size_clicked();

private:
    Ui::MainWindow *ui;
    UDPClient client;

    QRegExpValidator validator_ipv4;
    QIntValidator *validator_port;
    QIntValidator *validator_size;

    bool connected_local;
    bool connected_remote;

    bool check_port(const quint16 &port);
    bool check_size(const uint &size);

};

#endif // MAINWINDOW_H
