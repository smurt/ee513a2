#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"

#include<QDebug>
#include <json-c/json.h>

MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->time  = 0;
    this->topic = "";
    this->mqttData = new Data();
    this->isPitchChecked = false;
    this->isRollChecked = false;
    this->isTempChecked = false;
    this->setWindowTitle("EE513 Assignment 2");
    // 3 graphs for 3 different measurements
    this->ui->customPlot->addGraph();
    this->ui->customPlot->addGraph();
    this->ui->customPlot->addGraph();
    this->ui->customPlot->yAxis->setLabel("Degrees");
    ui->customPlot->graph(0)->setPen(QPen(Qt::red));
    ui->customPlot->graph(1)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(2)->setPen(QPen(Qt::green));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-180,180);
    this->ui->customPlot->replot();
    QObject::connect(this, SIGNAL(messageSignal(QString)),
                     this, SLOT(on_MQTTmessage(QString)));
    ::handle = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    // For more help on real-time plots, see: http://www.qcustomplot.com/index.php/demos/realtimedatademo
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds

    if (this->isPitchChecked) {
        ui->customPlot->graph(0)->addData(key,mqttData->getPitch());
    }
    if (this->isRollChecked) {
        ui->customPlot->graph(1)->addData(key,mqttData->getRoll());
    }
    if (this->isTempChecked) {
        ui->customPlot->graph(2)->addData(key,mqttData->getTemp());
    }

    ui->customPlot->graph(0)->rescaleKeyAxis(true);
    ui->customPlot->replot();
    //QString text = QString("Value added is %1").arg(this->count);
    //ui->outputEdit->setText(text);
}

void MainWindow::on_connectButton_clicked()
{
    // convert QString payload to char array
    QByteArray ba = (ui->inputTopicBox->toPlainText()).toLocal8Bit();
    this->topic = ba.data();
    ui->outputText->appendPlainText(QString(this->topic));

    if (strcmp(this->topic, "<write topic name here>") == 0 ||
            strcmp(this->topic, "") == 0) {
        ui->outputText->appendPlainText(QString("No topic specified. Cannot connect."));
        return;
    }

    // complete connection to topic
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic ") + this->topic + QString(" for client " CLIENTID));
    int x = MQTTClient_subscribe(client, this->topic, QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    // Please don't modify the Window UI from here
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

/* This is a callback function and is essentially another thread. Do not modify the
 * main window UI from here as it will cause problems. Please see the Slot method that
 * is directly below this function. To ensure that this method is thread safe I had to
 * get it to emit a signal which is received by the slot method below */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;
    QString payload;
    payload.sprintf("%s", (char *) message->payload).truncate(message->payloadlen);
    emit handle->messageSignal(payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/** This is the slot method. Do all of your message received work here. It is also safe
 * to call other methods on the object from this point in the code */
void MainWindow::on_MQTTmessage(QString payload){
    ui->outputText->appendPlainText(payload);
    ui->outputText->ensureCursorVisible();

    // convert QString payload to char array
    QByteArray ba = payload.toLocal8Bit();
    const char *charPayload = ba.data();

    // using json-c library to parse json message
     struct json_object *parsed_json;
     struct json_object *time;
     struct json_object *temp;
     struct json_object *accelerometer;
     struct json_object *rawX;
     struct json_object *rawY;
     struct json_object *rawZ;

     parsed_json = json_tokener_parse(charPayload);
     json_object_object_get_ex(parsed_json, "CurrentTime", &time);
     json_object_object_get_ex(parsed_json, "CPUTemp", &temp);
     json_object_object_get_ex(parsed_json, "Accelerometer", &accelerometer);
     json_object_object_get_ex(accelerometer, "X", &rawX);
     json_object_object_get_ex(accelerometer, "Y", &rawY);
     json_object_object_get_ex(accelerometer, "Z", &rawZ);

     ui->outputText->appendPlainText(QString("Current Time: ") + json_object_get_string(time));

     this->mqttData->setTemp( json_object_get_int(temp));
     this->mqttData->setPitch(json_object_get_int(rawX),
                              json_object_get_int(rawY),
                              json_object_get_int(rawZ));
     this->mqttData->setRoll( json_object_get_int(rawX),
                              json_object_get_int(rawY),
                              json_object_get_int(rawZ));

     ui->outputText->appendPlainText(QString("CPU Temp: ") +
                                     QString::number(this->mqttData->getTemp()) +
                                     QString(" degrees"));
     ui->outputText->appendPlainText(QString("Device Pitch: ") +
                                     QString::number(this->mqttData->getPitch()) +
                                     QString(" degrees"));
     ui->outputText->appendPlainText(QString("Device Roll: ")  +
                                     QString::number(this->mqttData->getRoll())  +
                                     QString(" degrees"));

     this->update();
}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    // Please don't modify the Window UI from here
    qDebug() << "Connection Lost" << endl;
}

void MainWindow::on_disconnectButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}

void MainWindow::on_checkBoxTemp_toggled(bool checked)
{
    this->isTempChecked = checked;
}

void MainWindow::on_checkBoxPitch_toggled(bool checked)
{
    this->isPitchChecked = checked;
}

void MainWindow::on_checkBoxRoll_toggled(bool checked)
{
    this->isRollChecked = checked;
}
