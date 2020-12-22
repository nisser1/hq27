#ifndef UIDEMO01_H
#define UIDEMO01_H
extern "C" {
#include "msg.h"
#include "service.h"
#include "uart245.h"
}
#include "netthread.h"
#include "ft2000bit.h"
#include <QDialog>

namespace Ui {
class UIDemo01;
}

class UIDemo01 : public QDialog
{
    Q_OBJECT

public:
    explicit UIDemo01(QWidget *parent = 0);
    ~UIDemo01();
    void checkChnInfo(int chnInfo);
    void checkFT2000bit();
protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::UIDemo01 *ui;

	netThread *m_netTread;
    FT2000BIT *m_ft2000thread;

signals:
	void setDisLight(int);
	void setDisContrst(int);
private slots:
    void initForm();
    void buttonClick();
	void slot_checkMessage(SERIAL_DATA msg);
private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();
	void on_pushButtonDisContrase_clicked();
	void on_pushButtonDisLight_clicked();
    void slot_RefFT2000HiBit();
    void slot_RefFT2000HiSys();
};

#endif // UIDEMO01_H
