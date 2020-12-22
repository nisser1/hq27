#include "ft2000bit.h"

FT2000BIT::FT2000BIT()
{
    fd = openCom("AMA1",115200,8,'N',1);
    if(fd < 0)
    {
        printf("open error\n");
    }
    printf("m_ft2000thread->fd=%d\n",fd);
}
FT2000BIT::~FT2000BIT()
{

}

void FT2000BIT::run()
{
    //QMap<QString, float> tem_map;
    int i= 0;
    int ret = 0;
    while(1)
    {
        printf("dddddddddVOLTAGE_MAX = %d\n",VOLTAGE_MAX);
        for(i = 0;i<=VOLTAGE_MAX;i++)
        {
            printf("dddddddddi=%d\n",i);
            float voltage;
            printf("m_ft2000thread->fd=%d\n",fd);
            ret = getVoltage(fd,&voltage,i);
            printf("1dddddddddi=%d\n",i);
            if(ret != 0)
            {
                switch (ret)
                {
                    case -1:printf("posit invalid\n");break;
                    case -2:printf("timeout\n");break;
                    case -3:printf("data format error");break;
                    default:
                        break;
                }
                continue;
            }

            switch (i)
            {
                case VOL_3_3:printf("3.3V电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("3.3Vvol",voltage);
                    break;
                case VOL_1_2:printf("1.2V电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.2Vvol",voltage);
                    break;
                case VOL_2_5:printf("2.5V电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("2.5Vvol",voltage);
                    break;
                case VOL_GPU:printf("1.1V_GPU电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.1Vvol",voltage);
                    break;
                case VOL_1_0:printf("1.0V电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.0Vvol",voltage);
                    break;
                case VOL_DDR:printf("1.2V_DDR电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.2Vvol",voltage);
                    break;
                case VOL_GPU_1_5:printf("1.5V_GPU电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.5Vvol",voltage);
                    break;
                case VOL_GPU_5_0:printf("5.0V_GPU电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("5.0Vvol",voltage);
                    break;
                case VOL_STB_3_3:printf("3.3V_STB电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("3.3STBVvol",voltage);
                    break;
                case VOL_STB_5_0:printf("1.8V_STB电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("5.0STBVvol",voltage);
                    break;
                case VOL_1_8:printf("1.8V电压ADC检测 = %.2f V\n",voltage);
                    tem_map.insert("1.8Vvol",voltage);
                    break;
                case VOL_CORE:printf("0.8V_CORE电压检测 = %.2f V\n",voltage);
                    tem_map.insert("0.8Vvol",voltage);
                    break;
                case CUR_3_0:printf("\n\n3.0A_电流检测 = %.2f A\n\n\n",voltage);
                    tem_map.insert("3.0A",voltage);
                    break;
                default:
                    break;
            }

        }




        for(i = 0;i<=TEMP_MAX;i++)
        {
            float temp;
            ret = getTemp(fd,&temp,i);
            if(ret != 0)
            {
                switch(ret)
                {
                    case -1:printf("posit invalid\n");
                    case -2:printf("timeout\n");
                    case -3:printf("data format error");
                    default:
                        break;
                }
                continue;
            }
            switch (i)
            {
                case TEMP_PCB:printf("PCB温度检测 = %.2f °C\n",temp);
                    tem_map.insert("PCBTEMP",temp);
                    break;
                case TEMP_GPU:printf("GPU温度检测 = %.2f °C\n",temp);
                    tem_map.insert("GPUTEMP",temp);
                    break;
                case TEMP_CPU:printf("CPU温度检测 = %.2f °C\n",temp);
                    tem_map.insert("CPUTEMP",temp);
                    break;
                default:
                    break;
            }
        }


        emit sig_FT200bit();
        sleep(1);
    }
}
