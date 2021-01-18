1:寄存器地址映射  又称之为应用层驱动 配置GPIO端口
   /dev/mem。发现功能非常奇妙，通过mmap能够将物理地址映射到用户空间的虚拟地址上。在用户空间完毕对设备寄存器的操作，于是上网搜了一些/dev/mem的资料。
网上的说法也非常统一，/dev/mem是物理内存的全映像，能够用来訪问物理内存，一般使用方法是open("/dev/mem",O_RDWR|O_SYNC)，接着就能够用mmap来訪问物理内存以及外设的IO资源，这就是实现用户空间驱动的一种方法。
    用户空间驱动听起来非常酷。可是对于/dev/mem，我认为没那么简单，有2个地方引起我的怀疑：
        （1）网上资料都说/dev/mem是物理内存的全镜像。这个概念非常含糊，/dev/mem究竟能够完毕哪些地址的虚实映射？
        （2）/dev/mem看似非常强大。可是这也太危急了，黑客全然能够利用/dev/mem对kernel代码以及IO进行一系列的非法操作，后果不可预測。难道内核开发人员们没有意识到这点吗?
 

2：利用/sys/class/gpio配置gpio 高还是低
    1: 首先先看系统中有没有 /sys/class/gpio 如果没有在内核编译的时候应该加入
            Device Drivers-> GPIO Support ->/sys/class/gpio/… (sysfs interface)
    2：gpio_operation 通过/sys/文件接口操作IO端口GPIO文件到文件系统的映射
        控制GPIO的端口位于/sys/class/gpio
        /sys/class/gpio/export 文件用于通知系统需要导出控制的GPIO引脚编号
        /sys/class/gpio/unexport 用于通知系统取消导出
        /sys/class/gpio/gpiochipX目录保存系统中GPIO寄存器的信息，包括每个寄存器控制引脚的其实编号base，寄存器名称，引脚总数，
        导出一个引脚的操作步骤
            首先先计算引脚编号 引脚编号=控制引脚的寄存器基数+控制引脚寄存器位数
                
            例子
                1: 导出
                    cd  /sys/class/gpio/
                    echo 44 > export
                2:设置方向
                    echo out > direction
                3:查看方向
                    cat direction
                4:设置输出
                    echo 1  > value
                5: 查看输出值
                    cat value
                6: 取消导出
               

