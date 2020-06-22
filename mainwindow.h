#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QFile>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    static constexpr quint8 code_info =0;
    static constexpr quint8 code_ready_for_data =1;
    static constexpr quint8 code_data =2;
    static constexpr quint8 code_end_file =3;
    static constexpr quint8 code_final =4;
    QTcpSocket* _socket = nullptr;
    QTcpServer* _server = nullptr;
    quint16 _port = 2121;
    QFile* file = nullptr;
    QFile file_client;
    quint64 blocksize = 0;
    int total_size = 0;
    qint64 size = 0;
    int temp_size = 0;
    static constexpr int size_part =1024;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_listenButton_clicked();
private slots:
   void myNewConnection();
   void myReadyRead();
   void myConnected();
   void on_sendButtom_clicked();

   void on_connectButton_clicked();

private:
    void SendAnswer(const quint8 code);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
