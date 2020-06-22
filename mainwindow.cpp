#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
    if (file)
        if (file->isOpen()) file->close();
    if (file_client.isOpen()) file_client.close();
}


void MainWindow::on_listenButton_clicked()
{
    setWindowTitle("Server");
    ui->textEdit->append("Run as TCP server ");
    _server = new QTcpServer(this);
    _server->listen(QHostAddress::Any,_port);
    connect(_server,SIGNAL(newConnection()),SLOT(myNewConnection()));
}

void MainWindow::myNewConnection()
{
    if (!_server) return;
    ui->textEdit->append("Someone connected to you ");
    _socket = _server->nextPendingConnection();
    connect(_socket,SIGNAL(readyRead()),SLOT(myReadyRead()));
}

void MainWindow::myReadyRead()
{
    if (!_socket) return;
    QDataStream in(_socket);
    in.setVersion(QDataStream::Qt_5_11);
    if (blocksize==0)
    {
        if (_socket->bytesAvailable()<(int)sizeof(quint64)) return;
        in>>blocksize;
    }
    if ((quint64)_socket->bytesAvailable()<blocksize) return;
    quint8 command;
    in>>command;
    switch (command)
    {
    case code_info: //info
    {
        QString fileName;
        total_size =0;
        in >> fileName>>size;
        ui->progressBar->setMaximum(size);
        ui->progressBar->show();
        file = new QFile(fileName);
        ui->textEdit->append(" You receive name file  : "+fileName +" with size = "+QVariant(size).toString());
        if (!file->open(QIODevice::WriteOnly))
        {
            qDebug() << "Can't open file for written";
            return;
        }
        SendAnswer(code_ready_for_data);

    }
        break;
    case code_ready_for_data: //ready for data
    {
        ui->textEdit->append(" Server answer, that he ready for receiving file  ");

        if (!file_client.atEnd())
        {
            QByteArray part;
            part.clear();
            part = file_client.read(size_part);
            temp_size+= part.size();
            ui->progressBar->setValue(temp_size);
            QByteArray block;
            block.clear();
            QDataStream out(&block,QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_5_11);
            out<<(quint64)0;//размер блока пока не знаем
            out<<code_data;//команда 2
            out<<part;
            out.device()->seek(0);
            out<<(quint64)(block.size()-sizeof(quint64));
            _socket->write(block);
            _socket->flush();
            ui->textEdit->append("Send to server  message with type 2: (data message) and size : "+QVariant(part.size()).toString());
        }
        else SendAnswer(code_end_file);
    }
        break;
    case code_data: //data
    {
        QByteArray buf;
        in>>buf;
        ui->textEdit->append(" Receive from clinent data message with size "+ QVariant(buf.size()).toString());
        total_size+=buf.size();
        ui->progressBar->setValue(total_size);
        if (total_size<=size)
        {
            if (file->isOpen())
            {
                file->write(buf);
                SendAnswer(code_ready_for_data);
            }
        }
    }
        break;
    case code_end_file: //end data
    {
        ui->progressBar->hide();
        ui->textEdit->append(" we receives all data ");
        if (file->isOpen()) file->close();
        SendAnswer(code_final);
    }
        break;
    case code_final: //end data
    {
        ui->progressBar->hide();
        if (file_client.isOpen()) file_client.close();
        ui->textEdit->append(" server said, that received all data ");
    }
        break;


    }

    blocksize=0;
}

void MainWindow::myConnected()
{
    ui->textEdit->append("Connected with server");
}


void MainWindow::on_sendButtom_clicked()
{
    QString path = QFileDialog::getOpenFileName();
    QFile file(path);
    QFileInfo info_file(file);
    file_client.setFileName(path);
    if (!file_client.open(QFile::ReadOnly)) return;

    temp_size = 0;
    ui->progressBar->setMaximum(file.size());
    ui->progressBar->show();

    QByteArray block;
    block.clear();
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    out<<(quint64)0;


    out << code_info<<info_file.fileName()<<info_file.size();
    out.device()->seek(0);
    out<<(quint64)(block.size()-sizeof(quint64));

    if (!_socket) return;
    _socket->write(block);
    _socket->flush();

}

void MainWindow::on_connectButton_clicked()
{
    setWindowTitle("Client");
    ui->textEdit->append("Run as TCP clinent ");
    _socket = new QTcpSocket(this);
    _socket->connectToHost(ui->line_ip->text(),_port);
    connect(_socket,SIGNAL(connected()),SLOT(myConnected()));
    connect(_socket,SIGNAL(readyRead()),SLOT(myReadyRead()));
    connect(_socket,SIGNAL(disconnected()),SLOT(deletelater()));

}

void MainWindow::SendAnswer(const quint8 code)
{
    QByteArray block;
    block.clear();
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    out<<(quint64)0;//размер блока пока не знаем
    out<<code;//команда code
    out.device()->seek(0);
    out<<(quint64)(block.size()-sizeof(quint64));
    _socket->write(block);
    _socket->flush();
    ui->textEdit->append("Send answer "+QVariant(code).toString());
}


