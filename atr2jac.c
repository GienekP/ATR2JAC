/*--------------------------------------------------------------------*/
/* ATR2JACflash4MbMax                                                 */
/* by GienekP                                                         */
/* (c) 2022                                                           */
/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
/*--------------------------------------------------------------------*/
#define RAMPROC 0x0100
/*--------------------------------------------------------------------*/
#define FLASHAUTO 0
#define FLASH128K 1
#define FLASH256K 2
#define FLASH512K 3
#define SEC128 1
#define SEC256 2
#define OLDADDR 0
#define NEWADDR 1
#define ORYGDOS 0
#define PATCHDOS 1
/*--------------------------------------------------------------------*/
#define CAR128 (128*1024)
#define CAR256 (256*1024)
#define CAR512 (512*1024)
#define CARMAX (1024*1024)
#define LDRSIZE (8*1024)
#define CLONESIZE (256)
#define ATRMAX (CARMAX-LDRSIZE-CLONESIZE*(CARMAX/8192))
/*--------------------------------------------------------------------*/
#include "starter128j.h"
#include "starter256j.h"
/*--------------------------------------------------------------------*/
void checkATR(const U8 *data, U8 *sector, unsigned int *type)
{
	if ((data[0]==0x96) && (data[1]==02))
	{
		if ((data[4]==0x80) && (data[5]==0x00))
		{
			*sector=SEC128;
		}
		else if ((data[4]==0x00) && (data[5]==0x01))
		{
			*sector=SEC256;
			if (data[2]&0x0F)
			{
				*type=3*128;
			}
			else
			{
				*type=6*128;
			};
		};
	};
}
/*--------------------------------------------------------------------*/
unsigned int loadATR(const char *filename, U8 *data, U8 *sector, unsigned int *type)
{
	U8 header[16];
	unsigned int ret=0;
	int i;
	FILE *pf;
	for (i=0; i<ATRMAX; i++)
	{
		data[i]=0xFF;
	};
	pf=fopen(filename,"rb");
	if (pf)
	{
		i=fread(header,sizeof(U8),16,pf);
		if (i==16)
		{
			checkATR(header,sector,type);
			if ((*sector==SEC128) || (*sector==SEC256)) 
			{
				ret=fread(data,sizeof(U8),ATRMAX,pf);
			}
			else
			{
				printf("Wrong sector header.\n");
			};
		}
		else
		{
			printf("Wrong ATR header size.\n");
		}
		fclose(pf);
	}
	else
	{
		printf("\"%s\" does not exist.\n",filename);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int assignFlash(unsigned int atrsize, U8 *flash)
{
	unsigned int ret;
	if (*flash==FLASHAUTO)
	{
		printf("CAR: Format 75 (0x4B) Atarimax 1 MB Flash cartridge (new)\n");		
	}
	else
	{
		printf("Forced ");
	};
	switch (*flash)
	{
		case FLASH128K: {printf("Flash: 128kB\n");ret=CAR128;} break;
		case FLASH256K: {printf("Flash: 256kB\n");ret=CAR256;} break;
		case FLASH512K: {printf("Flash: 512kB\n");ret=CAR512;} break;
		default: {printf("Flash AUTO\n");ret=CARMAX;} break;	
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void replace(U8 mode, U8 *atrdata, unsigned int i)
{
	static U8 f=0;
	if (mode==NEWADDR)
	{
		atrdata[i+1]=((RAMPROC)&0xFF);
		atrdata[i+2]=(((RAMPROC)>>8)&0xFF);
	};
	if (f==0)
	{
		f=1;
		if (mode==NEWADDR)
		{
			printf("Replace calls:\n");
		}
		else
		{
			printf("Possible calls:\n");
		};
	};
}
/*--------------------------------------------------------------------*/
void checkXINT(U8 *atrdata, U8 mode)
{
	unsigned int i;
	for (i=0; i<ATRMAX-3; i++)
	{
		if ((atrdata[i+1]==0x53) && (atrdata[i+2]==0xE4))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR JDSKINT ; 0x%06X 20 53 E4 -> 20 00 01\n",i+16);
				
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP JDSKINT ; 0x%06X 4C 53 E4 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0xB3) && (atrdata[i+2]==0xC6))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR DSKINT ; 0x%06X 20 B3 C6 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP DSKINT ; 0x%06X 4C B3 C6 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0x59) && (atrdata[i+2]==0xE4))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR JSIOINT ; 0x%06X 20 59 E4 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP JSIOINT ; 0x%06X 4C 59 E4 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0x33) && (atrdata[i+2]==0xC9))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR SIOINT ; 0x%06X 20 33 C9 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP SIOINT ; 0x%06X 4C 3 C9 -> 4C 00 01\n",i+16);
			};			
		};	
	};
}
/*--------------------------------------------------------------------*/
void repairDOSIId(U8 *atrdata, U8 patch)
{
	const unsigned char search[7]={0x20,0xD0,0x1A,0x29,0x20,0xF0,0x02};
	const unsigned int siz=sizeof(search);
	unsigned int i,j;
	for (i=0; i<ATRMAX-siz; i++)
	{
		unsigned int test=1;
		for (j=0; j<siz; j++)
		{
			if (atrdata[i+j]!=search[j]) {j=siz; test=0;};
		};
		if (test==1)
		{
			printf("Find DOS II+/D on 256sectors disk");
			if (patch==PATCHDOS)
			{
				printf(" and add patch.\n");
				atrdata[i+5]=0;
			}
			else
			{
				printf("\n");
			};
			i=ATRMAX;
		};
	};
}
/*--------------------------------------------------------------------*/
unsigned int buildCar128(const U8 *loader, unsigned int stsize, 
                         const U8 *atrdata, unsigned int atrsize, 
                         U8 *cardata, unsigned int carsize)
{
	unsigned int i,j,k=LDRSIZE,l=0,sum=0;
	for (i=0; i<CARMAX; i++)
	{
		cardata[i]=0xFF;
	};
	for (i=0; i<LDRSIZE; i++)
	{
		cardata[i]=loader[i];
	};
	for (i=0; i<(CARMAX/8192); i++)
	{
		for (j=0; j<(LDRSIZE-CLONESIZE); j++)
		{
			if (l<atrsize)
			{
				cardata[k]=atrdata[l];
			};
			k++;
			l++;
		};
		for (j=0; j<CLONESIZE; j++)
		{
			cardata[k]=loader[LDRSIZE-CLONESIZE+j];
			k++;
		};
	};
	for (i=0; i<carsize; i++)
	{
		sum+=cardata[i];
	};
	return sum;
}
/*--------------------------------------------------------------------*/
unsigned int buildCar256(const U8 *loader, unsigned int stsize, 
                         const U8 *atrdata, unsigned int atrsize, 
                         U8 *cardata, unsigned int carsize, unsigned int type)
{
	unsigned int i,j,k=LDRSIZE,l=0,sum=0;
	for (i=0; i<CARMAX; i++)
	{
		cardata[i]=0xFF;
	};
	for (i=0; i<LDRSIZE; i++)
	{
		cardata[i]=loader[i];
	}
	for (i=0; i<3; i++)
	{
		for (j=0; j<128; j++)
		{
			cardata[k]=atrdata[l];
			k++;
			l++;
		};
		k+=128;
	};
	for (i=0; i<28; i++)
	{
		for (j=0; j<256; j++)
		{
			cardata[k]=atrdata[l];
			k++;
			l++;
		};
	};
	for (j=0; j<(CLONESIZE); j++)
	{
		cardata[k]=loader[LDRSIZE-CLONESIZE+j];
		k++;
	};	
	for (i=1; i<(CARMAX/8192); i++)
	{
		for (j=0; j<(LDRSIZE-CLONESIZE); j++)
		{
			if (l<atrsize)
			{
				cardata[k]=atrdata[l];
			};
			k++;
			l++;
		};
		for (j=0; j<CLONESIZE; j++)
		{
			cardata[k]=loader[LDRSIZE-CLONESIZE+j];
			k++;
		};
	};
	for (i=0; i<carsize; i++)
	{
		sum+=cardata[i];
	};
	return sum;
}
/*--------------------------------------------------------------------*/
U8 saveCAR(const char *filename, U8 *data, unsigned int sum, U8 flash)
{
	U8 header[16]={0x43, 0x41, 0x52, 0x54, 0x00, 0x00, 0x00, 0x4B,
		           0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	U8 ret=0;
	int i,carsize;
	FILE *pf;
	switch (flash)
	{
		case FLASH128K: {carsize=CAR128;} break;
		case FLASH256K: {carsize=CAR256;} break;
		case FLASH512K: {carsize=CAR512;} break;
		default: {carsize=CARMAX;} break;
	};
	header[8]=((sum>>24)&0xFF);
	header[9]=((sum>>16)&0xFF);
	header[10]=((sum>>8)&0xFF);
	header[11]=(sum&0xFF);
	pf=fopen(filename,"wb");
	if (pf)
	{
		if (flash==FLASHAUTO)
		{
			i=fwrite(header,sizeof(U8),16,pf);
		}
		else
		{
			i=16;
		};
		if (i==16)
		{
			i=fwrite(data,sizeof(U8),carsize,pf);
			if (i==carsize)
			{
				ret=1;
			};			
		};
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void atr2jac(const char *atrfn, const char *carfn, U8 m, U8 f, U8 d)
{
	U8 atrdata[ATRMAX];
	U8 cardata[CARMAX];
	U8 mode=m;
	U8 flash=f;
	U8 dos=d;
	U8 sector;
	unsigned int atrsize;
	unsigned int carsize;
	unsigned int sum;
	unsigned int type;
	atrsize=loadATR(atrfn,atrdata,&sector,&type);
	if (atrsize)
	{
		printf("Load \"%s\"\n",atrfn);
		printf("ATR size: %i\n",atrsize+16);
		carsize=assignFlash(atrsize,&flash);
		if (((sector==SEC128) || (sector==SEC256)))
		{
			checkXINT(atrdata,mode);
			if (sector==SEC128) 
			{
				printf("Sector: 128\n");
				sum=buildCar128(starter128j_bin,starter128j_bin_len,atrdata,atrsize,cardata,carsize);
			}
			else
			{
				printf("Sector: 256\n");
				repairDOSIId(atrdata,dos);
				sum=buildCar256(starter256j_bin,starter256j_bin_len,atrdata,atrsize,cardata,carsize,type);
			};
			if (saveCAR(carfn,cardata,sum,flash))
			{
				printf("Save \"%s\"\n",carfn);
			}
			else
			{
				printf("Save \"%s\" ERROR!\n",carfn);
			};
		}
		else
		{
			printf("Sector: unsuported\n");
		};
	}
	else
	{
		printf("Load \"%s\" ERROR!\n",atrfn);
	};
}
/*--------------------------------------------------------------------*/
void flashSize(const char *str, U8 *flash)
{
	if ((str[0]=='-') && (str[1]=='1') && (str[2]=='2') && (str[3]=='8'))
	{
		*flash=FLASH128K;
	} else
	if ((str[0]=='-') && (str[1]=='2') && (str[2]=='5') && (str[3]=='6'))
	{
		*flash=FLASH256K;
	} else
	if ((str[0]=='-') && (str[1]=='5') && (str[2]=='1') && (str[3]=='2'))
	{
		*flash=FLASH512K;
	};	
}
/*--------------------------------------------------------------------*/
void modeSel(const char *str, U8 *mode)
{
	if ((str[0]=='-') && ((str[1]=='c') || (str[1]=='C')))
	{
		*mode=NEWADDR;
	};
}
/*--------------------------------------------------------------------*/
void patchDOS(const char *str, U8 *dos)
{
	if ((str[0]=='-') && ((str[1]=='d') || (str[1]=='D')))
	{
		*dos=PATCHDOS;
	};	
}
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{	
	U8 mode=OLDADDR;
	U8 flash=FLASHAUTO;
	U8 dos=ORYGDOS;
	printf("ATR2JAC - ver: %s\n",__DATE__);
	if (argc==3)
	{
		atr2jac(argv[1],argv[2],mode,flash,dos);
	}
	else if (argc==4)
	{
		modeSel(argv[3],&mode);
		flashSize(argv[3],&flash);
		patchDOS(argv[3],&dos);
		atr2jac(argv[1],argv[2],mode,flash,dos);
	}
	else if (argc==5)
	{
		modeSel(argv[3],&mode);
		flashSize(argv[3],&flash);
		patchDOS(argv[3],&dos);
		modeSel(argv[4],&mode);
		flashSize(argv[4],&flash);
		patchDOS(argv[4],&dos);
		atr2jac(argv[1],argv[2],mode,flash,dos);
	}
	else if (argc==6)
	{
		modeSel(argv[3],&mode);
		flashSize(argv[3],&flash);
		patchDOS(argv[3],&dos);
		modeSel(argv[4],&mode);
		flashSize(argv[4],&flash);
		patchDOS(argv[4],&dos);
		modeSel(argv[5],&mode);
		flashSize(argv[5],&flash);
		patchDOS(argv[5],&dos);
		atr2jac(argv[1],argv[2],mode,flash,dos);
	} else
	{
		printf("(c) GienekP\n");
		printf("use:\natr2jac file.atr file.car [-c] [-d] [-128|-256|-512]\n");
		printf("CAR  format \"75\" resized to Atarimax 1MB(new) for emulators\n");
		printf("-128  binary format for 128kB flash\n");
		printf("-256  binary format for 256kB flash\n");
		printf("-512  binary format for 512kB flash\n");
		printf("-c  remap JSR & JMP with JDSKINT / DSKINT\n");
		printf("-d  path DOS II+/D\n");		
	};
	printf("\n");
	return 0;
}
/*--------------------------------------------------------------------*/
