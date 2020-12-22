#ifndef NETTHREAD_H
#define NETTHREAD_H


extern "C" {
#include "msg.h"
}
#include <stdio.h>
#include <QThread>
#include <QUdpSocket>
#include <QSettings>
class netThread : public QThread
{
	Q_OBJECT

public:
	netThread();
	~netThread();
	void run();
	
	QUdpSocket *udpSock;
	QSettings *m_psetting;

	QString srcIpAddress;
	QString desIpAddress;
	int srcProt; 
	int desProt;
	int cmd;
	void printData(SERIAL_DATA msg);
signals:
	void recvNetMessage(SERIAL_DATA msg);
public slots:
	void slot_snedCmdMessage();
	void slot_snedDisLightCmd(int light);
	void slot_snedDisContrastCmd(int contrast);
private:
	
};

#endif // NETTHREAD_H
