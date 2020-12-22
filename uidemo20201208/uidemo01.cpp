#pragma execution_character_set("utf-8")

#include "uidemo01.h"
#include "ui_uidemo01.h"
#include "iconhelper.h"
#include <QString>
#include <iostream>
extern "C"{
#include "bit.h"
#include "msg.h"
}
using namespace std;
UIDemo01::UIDemo01(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UIDemo01)
{
    ui->setupUi(this);
    this->initForm();
	on_btnMenu_Max_clicked();
    this->setWindowFlags(Qt::FramelessWindowHint);

	m_netTread = new netThread();
	m_netTread->start();
    m_ft2000thread = new FT2000BIT();
    m_ft2000thread->start();

    connect(m_netTread, SIGNAL(recvNetMessage(SERIAL_DATA)), this, SLOT(slot_checkMessage(SERIAL_DATA)));
	connect(this,SIGNAL(setDisLight(int)), m_netTread,SLOT(slot_snedDisLightCmd(int)));
	connect(this, SIGNAL(setDisContrst(int)), m_netTread, SLOT(slot_snedDisContrastCmd(int)));
    connect(m_ft2000thread,SIGNAL(sig_FT200bit()), this, SLOT(slot_RefFT2000HiBit()));
    connect(m_ft2000thread,SIGNAL(sig_FT200bit()), this, SLOT(slot_RefFT2000HiSys()));
}

UIDemo01::~UIDemo01()
{
    delete ui;
}

bool UIDemo01::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched == ui->widgetTitle) {
            on_btnMenu_Max_clicked();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void UIDemo01::initForm()
{
    this->setProperty("form", true);
    this->setProperty("canMove", true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf099), 30);
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));

    //ui->widgetMenu->setVisible(false);
    ui->widgetTitle->installEventFilter(this);
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");
    ui->labTitle->setText("706健康管理平台");
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());

	//ui->stackedWidget->setStyleSheet("QLabel{font:20pt;}");
	ui->stackedWidget->setStyleSheet("QLineEdit{border-style:double;border-width:2;border-color:#ffffff;font:20pt;color white;}");
	//ui->stackedWidget->setStyleSheet("QGroupBox{font:20pt;}");


    //单独设置指示器大小
    int addWidth = 20;
    int addHeight = 10;
    int rbtnWidth = 15;
    int ckWidth = 13;
    int scrWidth = 12;
    int borderWidth = 3;

    QStringList qss;
    qss.append(QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::drop-down,QDateTimeEdit::drop-down{width:%1px;}").arg(addWidth));
    qss.append(QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                       "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%1px;height:%1px;right:2px;}").arg(addHeight));
    qss.append(QString("QRadioButton::indicator{width:%1px;height:%1px;}").arg(rbtnWidth));
    qss.append(QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::indicator,QListWidget::indicator{width:%1px;height:%1px;}").arg(ckWidth));
    qss.append(QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;}QScrollBar::handle:horizontal{border-radius:%2px;}"
                       "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}QScrollBar::handle:vertical{border-radius:%2px;}").arg(scrWidth).arg(scrWidth / 2));
    qss.append(QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_top>QToolButton:hover,"
                       "QWidget#widget_top>QToolButton:checked,QWidget#widget_top>QLabel:hover{"
                       "border-width:0px 0px %1px 0px;}").arg(borderWidth));
    qss.append(QString("QWidget#widgetleft>QPushButton:checked,QWidget#widgetleft>QToolButton:checked,"
                       "QWidget#widgetleft>QPushButton:pressed,QWidget#widgetleft>QToolButton:pressed{"
                       "border-width:0px 0px 0px %1px;}").arg(borderWidth));
	//qss.append();
    this->setStyleSheet(qss.join(""));


    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    ui->btnMain->click();

	//设置qlineedit不可编辑不可获得焦点

	ui->lineEditContrase->setEnabled(false);
	ui->lineEditDis12v->setEnabled(false);
	ui->lineEditDisBoard3_3->setEnabled(false);
	ui->lineEditDisBoard5v->setEnabled(false);
	ui->lineEditDisBoardTemp->setEnabled(false);
	ui->lineEditDisBorad1_2->setEnabled(false);
	ui->lineEditDisChn->setEnabled(false);
	ui->lineEditDisChnSig->setEnabled(false);
	ui->lineEditDisHeat->setEnabled(false);
	ui->lineEditDisLight->setEnabled(false);
	ui->lineEditDisTempSensor1->setEnabled(false);
	ui->lineEditDisTempSensor2->setEnabled(false);

	ui->lineEditContrase->setAlignment(Qt::AlignHCenter);
	ui->lineEditDis12v->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisBoard3_3->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisBoard5v->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisBoardTemp->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisBorad1_2->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisChn->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisChnSig->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisHeat->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisTempSensor1->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisTempSensor2->setAlignment(Qt::AlignHCenter);
	ui->lineEditDisLight->setAlignment(Qt::AlignHCenter);





    ui->lineEdithI5v->setEnabled(false);
    ui->lineEditHI2_5->setEnabled(false);
    ui->lineEditHi1_8->setEnabled(false);
    ui->lineEdithI0_9->setEnabled(false);
    ui->lineEditHiElect->setEnabled(false);
    ui->lineEditHi1_2v->setEnabled(false);
    ui->lineEditHiIcTemp->setEnabled(false);
    ui->lineEditHI12V->setEnabled(false);
    ui->lineEditHiPCBtemp->setEnabled(false);
    ui->lineEditHi3_3->setEnabled(false);
    ui->lineEditFT3_3->setEnabled(false);
    ui->lineEditFT1_2->setEnabled(false);
    ui->lineEditFT2_5->setEnabled(false);
    ui->lineEditFT1_1->setEnabled(false);
    ui->lineEditFT1_0->setEnabled(false);
    ui->lineEditFT1_2->setEnabled(false);
    ui->lineEditFT1_5->setEnabled(false);
    ui->lineEditFt0_8->setEnabled(false);
    ui->lineEditFT1_8->setEnabled(false);
    ui->lineEditFT5->setEnabled(false);
    ui->lineEditFtCurrent->setEnabled(false);
    ui->lineEditFtPcbTemp->setEnabled(false);
    ui->lineEditFtGpuTemp->setEnabled(false);
    ui->lineEditFtCpuTemp->setEnabled(false);
    ui->lineEditFtMEMOcc->setEnabled(false);
    ui->lineEditFtCpuOcc->setEnabled(false);
    ui->lineEditDiskOcc->setEnabled(false);
    ui->lineEditMemSize->setEnabled(false);
     ui->lineEditFTStb5->setEnabled(false);
    ui->lineEditFTStb3_3->setEnabled(false);


    ui->lineEditFTStb3_3->setAlignment(Qt::AlignHCenter);
     ui->lineEdithI5v->setAlignment(Qt::AlignHCenter);
     ui->lineEditHI2_5->setAlignment(Qt::AlignHCenter);
     ui->lineEditHi1_8->setAlignment(Qt::AlignHCenter);
     ui->lineEdithI0_9->setAlignment(Qt::AlignHCenter);
     ui->lineEditHiElect->setAlignment(Qt::AlignHCenter);
     ui->lineEditHi1_2v->setAlignment(Qt::AlignHCenter);
     ui->lineEditHiIcTemp->setAlignment(Qt::AlignHCenter);
     ui->lineEditHI12V->setAlignment(Qt::AlignHCenter);
     ui->lineEditHiPCBtemp->setAlignment(Qt::AlignHCenter);
     ui->lineEditHi3_3->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT3_3->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_2->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT2_5->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_1->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_0->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_2->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_5->setAlignment(Qt::AlignHCenter);
     ui->lineEditFt0_8->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT1_8->setAlignment(Qt::AlignHCenter);
     ui->lineEditFT5->setAlignment(Qt::AlignHCenter);

     ui->lineEditFtCurrent->setAlignment(Qt::AlignHCenter);
     ui->lineEditFtPcbTemp->setAlignment(Qt::AlignHCenter);
     ui->lineEditFtGpuTemp->setAlignment(Qt::AlignHCenter);
     ui->lineEditFtCpuTemp->setAlignment(Qt::AlignHCenter);
     ui->lineEditFtMEMOcc->setAlignment(Qt::AlignHCenter);
     ui->lineEditFtCpuOcc->setAlignment(Qt::AlignHCenter);
     ui->lineEditDiskOcc->setAlignment(Qt::AlignHCenter);
     ui->lineEditMemSize->setAlignment(Qt::AlignHCenter);


     ui->lineEditFTStb5->setAlignment(Qt::AlignHCenter);





    //ui->ht706DisLight->setText(QString::fromLocal8Bit("亮度"));

}

void UIDemo01::buttonClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();

    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        if (btn == b) {
            btn->setChecked(true);
        } else {
            btn->setChecked(false);
        }
    }

    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "设置") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "查询") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "用户退出") {
        exit(0);
    }
}


void UIDemo01::checkChnInfo(int chnInfo)
{
    switch(chnInfo)
    {


        case ALL_BOARDS_RF1: // 0
            break;
        case ALL_BOARDS_RF2: // 1
        case ALL_BOARDS_CVBS1: // 2
        case ALL_BOARDS_CVBS2: // 3
        case ALL_BOARDS_CVBS3: // 4
        case ALL_BOARDS_SVIDEO1: // 5
        case ALL_BOARDS_SVIDEO2: // 6
        case ALL_BOARDS_SVIDEO3: // 7
        case ALL_BOARDS_COMP1: // 8
        case ALL_BOARDS_COMP2: // 9
        case ALL_BOARDS_COMP3: // 10
        case ALL_BOARDS_SIG1_SCART1: // 11
        case ALL_BOARDS_SIG2_SCART1: // 12
        case ALL_BOARDS_SIG3_SCART1: // 13
        case ALL_BOARDS_SIG4_SCART1: // 14
        case ALL_BOARDS_SIG1_SCART2: // 15
        case ALL_BOARDS_SIG2_SCART2: // 16
        case ALL_BOARDS_SIG3_SCART2: // 17
        case ALL_BOARDS_SIG4_SCART2: // 18
        case ALL_BOARDS_SIG1_SCART3: // 19
        case ALL_BOARDS_SIG2_SCART3: // 20
        case ALL_BOARDS_SIG3_SCART3: // 21
        case ALL_BOARDS_SIG4_SCART3: // 22
            ui->lineEditDisChn->setText("未知");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_VGA1: // 23
            ui->lineEditDisChn->setText("VGA1");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_VGA2: // 24
            ui->lineEditDisChn->setText("VGA2");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_DVI1: // 25
            ui->lineEditDisChn->setText("DVI1");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_DIP1: // 26
            ui->lineEditDisChn->setText("DIP1");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_HDMI: // 27
            ui->lineEditDisChn->setText("HDMI1");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_HDMI2: // 28
            ui->lineEditDisChn->setText("HDMI2");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_HDMI3: // 29
            ui->lineEditDisChn->setText("HDMI3");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_DP1: // 30
            ui->lineEditDisChn->setText("DP1");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case ALL_BOARDS_DP2: // 31
            ui->lineEditDisChn->setText("DP2");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case NOT_USED: // 32ALL_BOARDS_SLAVE1: // 33
            ui->lineEditDisChn->setText("未知");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        case NUM_MASTER_LIST: // 34
            ui->lineEditDisChn->setText("未知");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
        default:
            ui->lineEditDisChn->setText("未知");
            ui->lineEditDisChn->setStyleSheet("color: white; ");
            break;
    }

}




//更新UI界面
void UIDemo01::slot_checkMessage(SERIAL_DATA msg)
{

	switch (msg.type)
	{
		case WRITE_LIGHT:
		case READ_LIGHT:
			//亮度
			if ((msg.light_value >= 0) && (msg.light_value <= 100))
			{
				ui->lineEditDisLight->setText(QString::number(msg.light_value));
				ui->lineEditDisLight->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisLight->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisLight->setStyleSheet("color: red; ");
			}
			
			break;
		case WRITE_CONTRAST:
		case READ_CONTRAST:
			//对比度
			if ((msg.contrast_value >= 0) && (msg.contrast_value <= 100) && (msg.type == READ_CONTRAST))
			{
				ui->lineEditContrase->setText(QString::number(msg.contrast_value));
				ui->lineEditContrase->setStyleSheet("color: white; ");
				
			}
			else
			{
				ui->lineEditContrase->setText(QString::fromLocal8Bit("0"));
				ui->lineEditContrase->setStyleSheet("color: red; ");

			}
			break;
		case BOARD_TEMP:
			//板卡温度
			if ((msg.board_temp >= 0) && (msg.board_temp <= 150) && (msg.type == BOARD_TEMP))
			{
				ui->lineEditDisBoardTemp->setText(QString::number(msg.board_temp));
				ui->lineEditDisBoardTemp->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisBoardTemp->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisBoardTemp->setStyleSheet("color: red; ");

			}
			break;
		case TEMP_SENSOR1:
			//温度传感器1
			if ((msg.temp_sersor1 >= 0) && (msg.temp_sersor1 <= 150) && (msg.type == TEMP_SENSOR1))
			{
				ui->lineEditDisTempSensor1->setText(QString::number(msg.temp_sersor1));
				ui->lineEditDisTempSensor1->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisTempSensor1->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisTempSensor1->setStyleSheet("color: red; ");

			}
			break;
		case TEMP_SENSOR2:
			//温度传感器2
			if ((msg.temp_sersor2 >= 0) || (msg.temp_sersor2 <= 150) && (msg.type == TEMP_SENSOR2))
			{
				ui->lineEditDisTempSensor2->setText(QString::number(msg.temp_sersor2));
				ui->lineEditDisTempSensor2->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisTempSensor2->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisTempSensor2->setStyleSheet("color: red; ");

			}
			break;
		case BOARD_5V:
			//5V板卡
			if ((msg.board_5v >= 0) || (msg.board_5v <= 8) && (msg.type == BOARD_5V))
			{
				ui->lineEditDisBoard5v->setText(QString::number(msg.board_5v));
				ui->lineEditDisBoard5v->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisBoard5v->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisBoard5v->setStyleSheet("color: red; ");
			}
			break;
		case BOARD_12V:
			//12V板卡
			if ((msg.board_12v >= 0) || (msg.board_12v <= 15) && (msg.type == BOARD_12V))
			{
				ui->lineEditDis12v->setText(QString::number(msg.board_12v));
				ui->lineEditDis12v->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDis12v->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDis12v->setStyleSheet("color: red; ");

			}
			break;
		case BOARD_3_3:
			//3.3V板卡
			if ((msg.board_3_3v >= 0) || (msg.board_3_3v <= 5) && (msg.type == BOARD_3_3))
			{
				ui->lineEditDisBoard3_3->setText(QString::number(msg.board_3_3v));
				ui->lineEditDisBoard3_3->setStyleSheet("color: white; ");
			}
			else
			{
				ui->lineEditDisBoard3_3->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisBoard3_3->setStyleSheet("color: red; ");

			}
			break;
		case BOARD_1_2:
			//1.2v板卡
			if ((msg.board_1_2v >= 0) || (msg.board_1_2v <= 2))
			{
				ui->lineEditDisBorad1_2->setText(QString::number(msg.board_1_2v));
				ui->lineEditDisBorad1_2->setStyleSheet("color: white; ");;
			}
			else
			{
				ui->lineEditDisBorad1_2->setText(QString::fromLocal8Bit("0"));
				ui->lineEditDisBorad1_2->setStyleSheet("color: red; ");

			}
			break;
		case GET_STATUS:
			//加热状态
			if ((msg.status & 0x01) == 1)
			{
                ui->lineEditDisHeat->setText("加热开启");
				ui->lineEditDisHeat->setStyleSheet("color: white; ");
			}
			else
			{
                ui->lineEditDisHeat->setText("加热关闭");
				ui->lineEditDisHeat->setStyleSheet("color: red; ");

			}

			//有无信号
			if ((msg.status & 0x02) == 2)
			{
                ui->lineEditDisChnSig->setText("有信号");
				ui->lineEditDisChnSig->setStyleSheet("color: white; ");
			}
			else
			{
                ui->lineEditDisChnSig->setText("无信号");
				ui->lineEditDisChnSig->setStyleSheet("color: red; ");

			}

			//通道接口  和status是一条信息
			if ((msg.current_chn >= 0) && (msg.current_chn <= 32))
			{

                checkChnInfo(msg.current_chn);
			}
			else
			{
                ui->lineEditDisChn->setText("无信号");
				ui->lineEditDisChn->setStyleSheet("color: red; ");

			}
			break;
		default:
			printf("recv msg error\n");
			break;
	}

}

void UIDemo01::on_btnMenu_Min_clicked()
{
    showMinimized();
}

void UIDemo01::on_btnMenu_Max_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
    }

    this->setProperty("canMove", max);
    max = !max;
}

void UIDemo01::on_btnMenu_Close_clicked()
{
    close();
}

void UIDemo01::on_pushButtonDisContrase_clicked()
{
		QString str = ui->lineEditDisSetContrase->text();

		//发送设置对比度的命令
		int contrast = str.toInt();

		if ((contrast <= 100) && (contrast > 0))
		{
			emit setDisContrst(contrast);
		}
}

void UIDemo01::on_pushButtonDisLight_clicked()
{
	QString str = ui->lineEditDisSetLight->text();

	//发送设置对比度的命令
	int light = str.toInt();

	if ((light <= 100) && (light > 0))
	{
		emit setDisLight(light);
    }
}

void UIDemo01::slot_RefFT2000HiBit()
{

    QMap<QString , float>::iterator it;
    for(it = m_ft2000thread->tem_map.begin(); it!= m_ft2000thread->tem_map.end(); ++it)
    {

        if(QString::fromLocal8Bit("3.3Vvol").compare(it.key()) == 0)
        {
  //          if((it.value() >= 3.96) || (it.value() <= 2.64))
 //           {
                ui->lineEditFT3_3->setText(QString::number(it.value()));
                ui->lineEditFT3_3->setStyleSheet("color: white; ");
 /*           }
            else
            {
                ui->lineEditFT3_3->setText(QString::number(it.value()));
                ui->lineEditFT3_3->setStyleSheet("color: red; ");
            }
*/
        }


        if(QString::fromLocal8Bit("1.2Vvol").compare(it.key()) == 0)
        {
 //           if((it.value() >= 1.44) || (it.value() <= 0.96))
  //          {
                 ui->lineEditFT1_2->setText(QString::number(it.value()));
                 ui->lineEditFT1_2->setStyleSheet("color: white; ");
 /*           }
            else
            {
                ui->lineEditFT1_2->setText(QString::number(it.value()));
                ui->lineEditFT1_2->setStyleSheet("color: red; ");
            }
   */     }

        if(QString::fromLocal8Bit("1.1Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT1_1->setText(QString::number(it.value()));
             ui->lineEditFT1_1->setStyleSheet("color: white; ");
        }

        if(QString::fromLocal8Bit("5.0STBVvol").compare(it.key()) == 0)
        {
             ui->lineEditFTStb5->setText(QString::number(it.value()));
             ui->lineEditFTStb5->setStyleSheet("color: white; ");
        }

        if(QString::fromLocal8Bit("1.0Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT1_0->setText(QString::number(it.value()));
             ui->lineEditFT1_0->setStyleSheet("color: white; ");
        }

        if(QString::fromLocal8Bit("1.5Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT1_5->setText(QString::number(it.value()));
             ui->lineEditFT1_5->setStyleSheet("color: white; ");
        }


        if(QString::fromLocal8Bit("5.0Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT5->setText(QString::number(it.value()));
             ui->lineEditFT5->setStyleSheet("color: white; ");
        }


        if(QString::fromLocal8Bit("3.3STBVvol").compare(it.key()) == 0)
        {
           ui->lineEditFTStb3_3->setText(QString::number(it.value()));
           ui->lineEditFTStb3_3->setStyleSheet("color: white; ");
        }
        if(QString::fromLocal8Bit("2.5Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT2_5->setText(QString::number(it.value()));
             ui->lineEditFT2_5->setStyleSheet("color: white; ");
        }
        if(QString::fromLocal8Bit("1.8Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFT1_8->setText(QString::number(it.value()));
             ui->lineEditFT1_8->setStyleSheet("color: white; ");
        }
        if(QString::fromLocal8Bit("0.8Vvol").compare(it.key()) == 0)
        {
             ui->lineEditFt0_8->setText(QString::number(it.value()));
             ui->lineEditFt0_8->setStyleSheet("color: white; ");
        }
        if(QString::fromLocal8Bit("3.0A").compare(it.key()) == 0)
        {
             ui->lineEditFtCurrent->setText(QString::number(it.value()));
             ui->lineEditFtCurrent->setStyleSheet("color: white; ");
        }


        if(QString::fromLocal8Bit("PCBTEMP").compare(it.key()) == 0)
        {
             ui->lineEditFtPcbTemp->setText(QString::number(it.value()));
             ui->lineEditFtPcbTemp->setStyleSheet("color: white; ");
        }

        if(QString::fromLocal8Bit("GPUTEMP").compare(it.key()) == 0)
        {
             ui->lineEditFtGpuTemp->setText(QString::number(it.value()));
             ui->lineEditFtGpuTemp->setStyleSheet("color: white; ");
        }

        if(QString::fromLocal8Bit("CPUTEMP").compare(it.key()) == 0)
        {
             ui->lineEditFtCpuTemp->setText(QString::number(it.value()));
             ui->lineEditFtCpuTemp->setStyleSheet("color: white; ");
        }
    }

}

void UIDemo01::slot_RefFT2000HiSys()
{
    CPU_OCCUPY cpu_stat1, cpu_stat2;
    int cpu;
    int ret;
    cpu=0;
    get_cpuoccupy1((CPU_OCCUPY*)&cpu_stat1);

    sleep(1);
    get_cpuoccupy1((CPU_OCCUPY*)&cpu_stat2);

    cpu = cal_cpuoccupy1((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
    ui->lineEditFtCpuOcc->setText(QString::number(cpu));
     ui->lineEditFtCpuOcc->setStyleSheet("color: white; ");



    //mem
    MEMInfo pMEMInfo;
    ML_MTMW_MEMBIT(1, &pMEMInfo);
    ui->lineEditFtMEMOcc->setText(QString::number(pMEMInfo.mem_used_perc));
    ui->lineEditFtMEMOcc->setStyleSheet("color: white; ");
    DISKInfo pDISKInfo;
    ML_MTMW_DISKBIT(1,&pDISKInfo);
    //printf("ddddddddddddddddddddddddddddd%d\n",pDISKInfo.disk_usage);
    ui->lineEditDiskOcc->setText(QString::number(pDISKInfo.disk_usage));
     ui->lineEditDiskOcc->setStyleSheet("color: white; ");


     //mem size
     ui->lineEditMemSize->setText("8GB");
     ui->lineEditMemSize->setStyleSheet("color: white; ");
}


void UIDemo01::checkFT2000bit()
{

}
