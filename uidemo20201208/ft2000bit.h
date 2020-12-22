#ifndef FT2000BIT_H
#define FT2000BIT_H
#include <QThread>
#include <QMap>
#include <QString>
extern "C"{
#include "/home/ht706/nisser/lib/uart245.h"
}
class FT2000BIT : public QThread
{
    Q_OBJECT




public:
    int fd;
    QMap <QString ,float > tem_map;

    FT2000BIT();

    ~FT2000BIT();
    void run();
signals:
    void sig_FT200bit();
};

#endif // FT2000BIT_H
