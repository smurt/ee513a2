#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "data.h"
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.0.11:1883"
#define CLIENTID    "Qt Application"
#define AUTHMETHOD  "sinead"
#define AUTHTOKEN   "murtagh"
//#define TOPIC       "ee513/test"
#define PAYLOAD     "Hi from Qt!"
#define QOS         1
#define TIMEOUT     10000L

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    char *topic;

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_MQTTmessage(QString message);

    void on_checkBoxTemp_toggled(bool checked);
    void on_checkBoxPitch_toggled(bool checked);
    void on_checkBoxRoll_toggled(bool checked);

signals:
    void messageSignal(QString message);

private:
    Ui::MainWindow *ui;
    void update();
    int time;
    Data *mqttData;
    bool isPitchChecked, isRollChecked, isTempChecked;
    MQTTClient client;
    volatile MQTTClient_deliveryToken deliveredtoken;

    friend void delivered(void *context, MQTTClient_deliveryToken dt);
    friend int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    friend void connlost(void *context, char *cause);
};

void delivered(void *context, MQTTClient_deliveryToken dt);
int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);

#endif // MAINWINDOW_H

