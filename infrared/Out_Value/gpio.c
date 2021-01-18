#include "gpio.h"

static int fd = -1;
static const char dev[]="/dev/mem";
TMMAP_Node_t * pTMMAPNode = NULL;
/* no need considering page_size of 4K */
static void * memmap(unsigned long phy_addr, unsigned long size)
{

	unsigned long phy_addr_in_page;
	unsigned long page_diff;

	unsigned long size_in_page;

	TMMAP_Node_t * pTmp;
	TMMAP_Node_t * pNew;

	void *addr=NULL;

	if(size == 0)
	{
		WRITE_LOG_ERROR("memmap():size can't be zero!\n");
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pTMMAPNode;
	while(pTmp != NULL)
	{
		if( (phy_addr >= pTmp->Start_P) && 
				( (phy_addr + size) <= (pTmp->Start_P + pTmp->length) ) )
		{
			pTmp->refcount++;   /* referrence count increase by 1  */
			return (void *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
		}

		pTmp = pTmp->next;
	}

	/* not mmaped yet */
	if(fd < 0)
	{
		/* dev not opened yet, so open it */
		fd = open (dev, O_RDWR | O_SYNC);
		if (fd < 0)
		{
			WRITE_LOG_ERROR("memmap():open %s error!\n", dev);
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		WRITE_LOG_ERROR("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	/* add this mmap to MMAP Node */
	pNew = (TMMAP_Node_t *)malloc(sizeof(TMMAP_Node_t));
	if(NULL == pNew)
	{
		WRITE_LOG_ERROR("memmap():malloc new node failed!\n");
		return NULL;
	}
	pNew->Start_P = phy_addr_in_page;
	pNew->Start_V = (unsigned long)addr;
	pNew->length = size_in_page;
	pNew->refcount = 1;
	pNew->next = NULL;

	if(pTMMAPNode == NULL)
	{
		pTMMAPNode = pNew;
	}
	else
	{
		pTmp = pTMMAPNode;
		while(pTmp->next != NULL)
		{
			pTmp = pTmp->next;
		}

		pTmp->next = pNew;
	}
	return (void *)(addr+page_diff);

}


/*****************************************************************************
Prototype    : memunmap
Description  : 
Input        : void * addr_mapped  
Output       : None
Return Value : On success, returns 0, on failure -1 
Calls        : 
Called By    : 

History        :
1.Date         : 2005/12/21
Author       : Z42136
Modification : Created function

 *****************************************************************************/
static int memunmap(void * addr_mapped)
{
	TMMAP_Node_t * pPre;
	TMMAP_Node_t * pTmp;

	if(pTMMAPNode == NULL)
	{
		WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
		return -1;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pTMMAPNode;
	pPre = pTMMAPNode;

	do
	{
		if( ((unsigned long)addr_mapped >= pTmp->Start_V) && 
				((unsigned long)addr_mapped <= (pTmp->Start_V + pTmp->length)) )
		{
			pTmp->refcount--;   /* referrence count decrease by 1  */
			if(0 == pTmp->refcount)
			{

				WRITE_LOG_INFO("memunmap(): map node will be remove!\n");

				/* delete this map node from pMMAPNode */
				if(pTmp == pTMMAPNode)
				{
					pTMMAPNode = NULL;
				}
				else
				{
					pPre->next = pTmp->next;
				}

				/* munmap */
				if(munmap((void *)pTmp->Start_V, pTmp->length) != 0 )
				{
					WRITE_LOG_INFO("memunmap(): munmap failed!\n");
				}

				free(pTmp);
			}

			return 0;
		}

		pPre = pTmp;
		pTmp = pTmp->next;
	}while(pTmp != NULL);

	WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
	return -1;
}

int ztInitGpioMap(struct ztGpioPrivate *Private)
{
    int offset = 0;
    int value = 0;
    int i = 0;

    Private->LED1_Map = (volatile unsigned int *)memmap(GPIO2_0,0x04);
    Private->LED2_Map = (volatile unsigned int *)memmap(GPIO14_0,0x04);
	Private->LED3_Map = (volatile unsigned int *)memmap(GPIO14_1,0x04);
	Private->LED4_Map = (volatile unsigned int *)memmap(GPIO14_2,0x04);

    if (NULL == Private->LED1_Map || NULL == Private->LED2_Map ||
        NULL == Private->LED3_Map || NULL == Private->LED4_Map) {
        printf("NULL pointer!\n");
        return -1;
    }
    else {
        printf("Successfull!\n");
    }

#if 0
    printf("0x%x\r\n",Private->LED1_Map);
    printf("0x%x\r\n",Private->LED2_Map);
    printf("0x%x\r\n",Private->LED3_Map);
    printf("0x%x\r\n",Private->LED4_Map);
    printf("before+++++++++\r\n");
	printf("Private->Led%d_map = 0x%x\r\n", 1, Private->LED1_Map[0]);
    printf("Private->Led%d_map = 0x%x\r\n", 2, Private->LED2_Map[0]);
    printf("Private->Led%d_map = 0x%x\r\n", 3, Private->LED3_Map[0]);
	printf("Private->Led%d_map = 0x%x\r\n", 4, Private->LED4_Map[0]);
#endif   
   /* 将4个LED管脚初始化 */
    Private->LED1_Map[0] = 0x1600 | 0xf << 4 ;
    Private->LED2_Map[0] = 0x1600 | 0xf << 4;
    Private->LED3_Map[0] = 0x1600 | 0xf << 4;
    Private->LED4_Map[0] = 0x1600 | 0xf << 4;
#if 0
    printf("after+++++++++\r\n");
  	printf("Private->Led%d_map = 0x%x\r\n", 1, Private->LED1_Map[0]);
    printf("Private->Led%d_map = 0x%x\r\n", 2, Private->LED2_Map[0]);
	printf("Private->Led%d_map = 0x%x\r\n", 3, Private->LED3_Map[0]);
	printf("Private->Led%d_map = 0x%x\r\n", 4, Private->LED4_Map[0]);
#endif   
 
    return 0;
}

int ztReleaseGpioMap(struct ztGpioPrivate *Private)
{
    memunmap((void *)Private->LED1_Map);
    memunmap((void *)Private->LED2_Map);
    memunmap((void *)Private->LED3_Map);
    memunmap((void *)Private->LED4_Map);
}



int ztGpioExport(int number)
{
    int fd;
    char gpio_str[8] = {'\0'};
    int len;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        dbg_perror("open gpio export");
        return -1;
    }

    sprintf(gpio_str, "%d", number);
    len = strlen(gpio_str);
    if (write(fd, gpio_str, len) < len) {
        dbg_perror("write to export");
        goto _ERR;
    }

    close(fd);
    return 0;

_ERR:
    close(fd);
    return -1;
}

int ztGpioUnexport(int number)
{
    int fd;
    char gpio_str[8] = {'\0'};
    int len;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        dbg_perror("open gpio unexport");
        return -1;
    }

    sprintf(gpio_str, "%d", number);
    len = strlen(gpio_str);
    if (write(fd, gpio_str, len) < len) {
        dbg_perror("write to unexport");
        goto _ERR;
    }

    close(fd);
    return 0;

_ERR:
    close(fd);
    return -1;
}

int ztGpioSetDir(int number, int dir)
{
    int ret = 0;
    int fd = -1;
    char dir_path[MAX_INPUT] = {'\0'};
    char dir_verify[4] = {'\0'};
    char dir_write[4] = {'\0'};

    sprintf(dir_path, "/sys/class/gpio/gpio%d/direction", number);
    fd = open(dir_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio direction");
        return -1;
    }

    if (dir == GPIO_IN) {
        strcpy(dir_write, "in");
    }
    else if (dir == GPIO_OUT) {
        strcpy(dir_write, "out");
    }
    else {
        return -2;
    }

    ret = write(fd, dir_write, strlen(dir_write));
    if (ret < 0) {
        dbg_perror("write gpio direction");
        goto _ERR;
    }

    close(fd);

    fd = open(dir_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio value");
        return -1;
    }

    ret = read(fd, dir_verify, strlen(dir_write));
    if (ret < 0) {
        dbg_perror("read direction back");
        goto _ERR;
    }


    if (strncmp(dir_write, dir_verify, strlen(dir_write)) != 0) {
        dbg_printf("set direction failed\n");
        goto _ERR;
    }

    close(fd);
    return 0;

_ERR:
    close(fd);
    return -1;
}

int ztGpioGetDir(int number, char *dir)
{
    int ret = 0;
    int fd = -1;
    char dir_path[MAX_INPUT] = {'\0'};
    char dir_verify[4] = {0};

    memset(dir_verify, 0, sizeof(dir_verify));

    sprintf(dir_path, "/sys/class/gpio/gpio%d/direction", number);
    fd = open(dir_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio direction");
        return -1;
    }

    ret = read(fd, dir_verify, 3);
    if (ret < 0) {
        dbg_perror("read direction back");
        goto _ERR;
    }

    memcpy(dir, dir_verify, 3);
    close(fd);
    return 0;

_ERR:
    close(fd);
    return -1;
}

/******************************************************************************
  Function:       ztGpioSetLevel
  Description:    set the gpio level
  Input:          number    	--  gpio number integer value. eg.gpio3_7 =
3*32+7 level			--  set gpio level. 1: high; 0: low. Output:
  Return:         int			--  return the result
  Others:         NONE
*******************************************************************************/
int ztGpioSetLevel(int number, int level)
{
    int ret = 0;
    int fd;
    char value_path[MAX_INPUT] = {'\0'};
    char value;
    char value_verify;

    sprintf(value_path, "/sys/class/gpio/gpio%d/value", number);
    fd = open(value_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio value");
        return -1;
    }
    if (level == 1) {
        value = '1';
    }
    else if (level == 0) {
        value = '0';
    }

    write(fd, &value, 1);
    if (ret < 0) {
        dbg_perror("write gpio level");
        return -1;
    }

    close(fd);
    fd = open(value_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio value");
        return -1;
    }

    if (read(fd, &value_verify, 1) < 1) {
        dbg_perror("read value back");
        close(fd);
        return -1;
    }

    if ((value - value_verify) != 0) {
        dbg_printf("set value failed\n");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int ztGpioGetLevel(int number, char *level)
{
    int ret = 0;
    int fd;
    char value_path[MAX_INPUT] = {'\0'};

    sprintf(value_path, "/sys/class/gpio/gpio%d/value", number);
    fd = open(value_path, O_RDWR);
    if (fd < 0) {
        dbg_perror("open gpio value");
        return -1;
    }

    ret = read(fd, level, 1);
    if (ret < 1) {
        dbg_perror("read value back");
        close(fd);
        return -1;
    }

    return ret;
}
