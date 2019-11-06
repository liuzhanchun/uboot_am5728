#ifndef _LZC_CONFIG_H
#define _LZC_CONFIG_H

#define ENABLE_USB_UPDATA
//#undef ENABLE_USB_UPDATA

#define UPDATE_PATH_FOLDER "/mr3300a"
#define FILE_SAVE_DDR_ADDR 0xc2000000

struct s_init_file
{
	int nIndex;						/*索引2.5.3.6之后添加该索引*/
	const char *fileInfo;		    /*升级提示*/
	const char *fileName;		    /*升级文件名*/
	unsigned long ufileStartAddr;           /*起始地址*/
	unsigned long ufileSize;                /*大小*/
};
typedef struct s_init_file FILEINFO;


enum
{
	INDEX_UBL = 0,
	INDEX_UBOOT,
	INDEX_UBOOT_ENV,
	INDEX_KERNEL,
	INDEX_KERNEL_LOGO,
	INDEX_SA_DSP,
	INDEX_RFS,
	INDEX_LZC,
};


extern int usb_part_valid_num;
extern char usb_part[5];


#endif	/* _LZC_CONFIG_H */