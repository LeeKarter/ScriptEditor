#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QUdpSocket>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QUdpSocket(this);

    bool ok = socket->bind(45454, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if(!ok) {
        QMessageBox::warning(this, "Ошибка", "UDP соединение не установлено");
    }

    connect(socket, &QUdpSocket::readyRead, this, &MainWindow::onRequest);

    //стандартный скрипт
    ui->editor->setPlainText(
        "clear();\n"
        "drawFilledCircle(100,105,100);\n"
        "drawCircle(320,105,100);\n"
        "drawTriangle(430,15, 200, 170,'blue', 'red');\n"
        "drawRect(10,220,190,130, 'red','green');\n"
        "drawLine(220,230,630,230,'red');\n"
        "drawLine(220,265,630,265,'green');\n"
        "drawLine(220,300,630,300,'blue');\n"
        "drawLine(220,335,630,335,'magenta');\n"
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRequest() { //функция обработки входящих запросов
    while(socket->hasPendingDatagrams()) { //цикл обработки непрочитаных запросов
        QByteArray data;
        data.resize(socket->pendingDatagramSize()); //размер буфера под входящий пакет

        QHostAddress sender; //ip адрес отправителя
        quint16 senderPort; //сокет отправителя

        socket->readDatagram(data.data(), data.size(), &sender, &senderPort); //чтение пакета

        QString request = QString::fromUtf8(data).trimmed(); //преобразование в строку и удаление ненужных знаков
        if(request == "DISCOVER") { //клиент отправил DISCOVER
            socket->writeDatagram("SERVER_HERE", sender, senderPort); //ответ клиенту
        }
        else if(request == "GET_DATA") { //клиент запрашивает скрипт
            QByteArray response = "SCRIPT_BEGIN" + ui->editor->toPlainText().toUtf8(); //текст из редактора скрипта преобразовывается в байтовый формат и подставляется идентификатор скрипта
            socket->writeDatagram(response, sender, senderPort); //отправка скрипта клиенту
        }
    }
}

void MainWindow::on_loadButton_clicked() //нажата кнопка "Загрузить"
{
    QString name = QFileDialog::getOpenFileName(this, "Открыть", "", "*.qs"); //выбор файла со скриптом
    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) { //открытие файла для чтения
        ui->editor->setPlainText(f.readAll()); //чтение и вставка файла в редактор
    }
}

void MainWindow::on_saveButton_clicked() //нажата кнопка "Сохранить"
{
    QString name = QFileDialog::getSaveFileName(this, "Сохранить", "", "*.qs"); //окно для сохранения файла
    QFile f(name);
    if (f.open(QIODevice::WriteOnly)) { //открытие файла для записи
        f.write(ui->editor->toPlainText().toUtf8()); //запись скрипта в файл
    }
}

