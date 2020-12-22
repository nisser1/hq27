#include "netthread.h"
#include <iostream>
#include <QCoreApplication>
#include <QString>

using namespace std;
netThread::netThread()
{
	udpSock = new QUdpSocket(this);



	QString m_qstrFileName = QCoreApplication::applicationDirPath() + "/config.ini";
	cout<< m_qstrFileName.toStdString() << endl;
	m_psetting = new QSettings(m_qstrFileName, QSettings::IniFormat);
	
	//本机ip地址
	srcIpAddress = m_psetting->value("srcIpAdress/ip").toString();
	srcProt = m_psetting->value("srcPort/port").toInt();

	//海思ip地址
	desIpAddress = m_psetting->value("desIpAdress/ip").toString();
	desProt = m_psetting->value("desPort/port").toInt();

	cout << srcIpAddress.toStdString() <<" "<< srcProt << " " << desIpAddress.toStdString() << " " << desProt << endl;
	udpSock->bind(QHostAddress(srcIpAddress), srcProt, QUdpSocket::ShareAddress);
	connect(udpSock, SIGNAL(readyRead()), this, SLOT(slot_snedCmdMessage()));
}

netThread::~netThread()
{

}

void netThread::run()
{
		/*
		wl		WRITE_LIGHT,
		rl      READ_LIGHT,
		wc      WRITE_CONTRAST,
		rc      READ_CONTRAST,
		bt      BOARD_TEMP,
		t1      TEMP_SENSOR1,
		t2      TEMP_SENSOR2,
		b5      BOARD_5V,
		b2      BOARD_12V,
		b3      BOARD_3_3,
		b1      BOARD_1_2,
		bs      GET_STATUS
		*/
	while (1)
	{
		sleep(1);
		//读取亮度
		udpSock->writeDatagram("rl", 2, QHostAddress(desIpAddress), desProt);

		usleep(100000);
		//读取对比度
		udpSock->writeDatagram("rc", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//读取主板温度
		udpSock->writeDatagram("bt", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//温度传感器1温度
		udpSock->writeDatagram("t1", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//温度传感器2温度
		udpSock->writeDatagram("t2", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//主板5V
		udpSock->writeDatagram("b5", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//主板12V
		udpSock->writeDatagram("b2", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//主板3.3
		udpSock->writeDatagram("b3", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//主板1.2
		udpSock->writeDatagram("b1", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		//显示器状态
		udpSock->writeDatagram("bs", 2, QHostAddress(desIpAddress), desProt);
		usleep(100000);

		cout << "send end" << endl;
	}
}

	
void netThread::printData(SERIAL_DATA msg)
{
	printf("lightVal=%d contrastValue=%d boardTmp=%f tempSensor1=%f tempSensor2=%f board5V=%f borad12V=%f Board3.3=%f board1.2=%f disCurrentChn=%d status=%d type=%d errorType=%d ExecRes=%d\n", \
		msg.light_value, \
		msg.contrast_value, \
		msg.board_temp, \
		msg.temp_sersor1, \
		msg.temp_sersor2, \
		msg.board_5v, \
		msg.board_12v, \
		msg.board_3_3v, \
		msg.board_1_2v, \
		msg.current_chn, \
		msg.status, \
		msg.type, \
		msg.error_type, \
		msg.cmd_exec_result);

}

void netThread::slot_snedCmdMessage()
{
	//QByteArray ba;
	SERIAL_DATA msg;
	while(udpSock->hasPendingDatagrams())
	{
		udpSock->readDatagram((char *)&msg, sizeof(SERIAL_DATA));
		printData(msg);
		
		emit recvNetMessage(msg);
	}
}

void netThread::slot_snedDisLightCmd(int light)
{
	unsigned char lightTransformation = (unsigned char)((light / 100) * 255) ;
	QString str = "wl" + QString::number(light);
	char*  ch;
	QByteArray ba = str.toLatin1(); // must
	ch = ba.data();
	udpSock->writeDatagram(ch, 8, QHostAddress(desIpAddress), desProt);
}

void netThread::slot_snedDisContrastCmd(int contrast)
{
	QString str = "wc" + QString::number(contrast);
	char*  ch;
	QByteArray ba = str.toLatin1(); // must
	ch = ba.data();
	udpSock->writeDatagram(ch, 8, QHostAddress(desIpAddress), desProt);
}

