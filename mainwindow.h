#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFileDialog>

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
    void on_new_message(const Client &sender, const QString &message);
    void on_message_delivered(const Client &sender);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void on_connect_btn_clicked();

    void on_save_btn_clicked();

    void on_send_btn_clicked();

    void on_save_d_size_clicked();

    void on_save_interval_clicked();

    void on_file_btn_clicked();

private:
    Ui::MainWindow *ui;
    UDPClient client;

    QRegExpValidator validator_ipv4;
    QIntValidator *validator_int;

    bool connected_local;
    bool connected_remote;

    bool checkPort(const quint16 &port);
    bool checkSize(const uint &size);

};

#endif // MAINWINDOW_H
