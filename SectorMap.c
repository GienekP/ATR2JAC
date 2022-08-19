/*--------------------------------------------------------------------*/
/* SectorMap                                                          */
/* by GienekP                                                         */
/* (c) 2022                                                           */
/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
#define SECSIZE128 128
#define SECSIZE256 256
#define PROCSIZE128 256
#define PROCSIZE256 256
/*--------------------------------------------------------------------*/
const unsigned char maxpos128=((8192-PROCSIZE128)/SECSIZE128);
const unsigned int maxsec128=((8192-PROCSIZE128)*((512-1)*1024/8192)/SECSIZE128)-200;
const unsigned char maxpos256=((8192-PROCSIZE256)/SECSIZE256);
const unsigned int maxsec256=((8192-PROCSIZE256)*((1024-1)*1024/8192)/SECSIZE256)-256;
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{	
	int i;
	unsigned char bank=1;
	unsigned char pos=0;
	FILE *pf;
	printf("SectorMap-128 Generator\nMax Disk Size: %ibytes (%i sectors)\n",maxsec128*SECSIZE128,maxsec128);
	pf=fopen("SectorMap128.dta","wb");
	if (pf)
	{
		printf("Build SectorMap128.dta\n");
		fputc(0xFF,pf);
		fputc(0xFF,pf);
		for (i=1; i<=maxsec128; i++)
		{
			putc(bank,pf);
			putc(pos,pf);
			printf("Sector: %i -> Side: %X Bank: %X  Pos: %02X ($%04X 0x%05X)\n",i,(bank>>4)&3,bank&15,pos,SECSIZE128*pos+0xA000,bank*8192+SECSIZE128*pos);
			pos++;
			if (pos==maxpos128)
			{
				pos=0;
				bank++;
				bank&=0x3F;
			};
		};	
		fclose(pf);
	};
	bank=1;
	pos=0;
	printf("SectorMap-256 Generator\nMax Disk Size: %ibytes (%i sectors)\n",maxsec256*SECSIZE256,maxsec256);
	pf=fopen("SectorMap256.dta","wb");
	if (pf)
	{
		printf("Build SectorMap256.dta\n");
		fputc(0xFF,pf);
		fputc(0xFF,pf);
		for (i=1; i<=maxsec256; i++)
		{
			putc(bank,pf);
			putc(pos,pf);
			printf("Sector: %i -> Side: %X Bank: %X  Pos: %02X ($%04X 0x%05X)\n",i,(bank>>4)&3,bank&15,pos,SECSIZE256*pos+0xA000,bank*8192+SECSIZE256*pos);
			pos++;
			if (pos==maxpos256)
			{
				pos=0;
				bank++;
				bank&=0x3F;
			};
		};	
		fclose(pf);
	};/*
	printf("Bigest %isectors\n",maxsec256);
	pf=fopen("Bigest.atr","wb");
	if (pf)
	{
		unsigned char atrh[16]={0x96,0x02,0xE8,0x1E,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		fwrite(atrh,sizeof(unsigned char),sizeof(atrh),pf);
		for (i=0; i<128; i++) {fputc(0x01,pf);};
		for (i=0; i<128; i++) {fputc(0x02,pf);};
		for (i=0; i<128; i++) {fputc(0x03,pf);};		
		for (i=4*256; i<((maxsec256+1)*256); i++)
		{
			unsigned char m;
			m=((i>>8)&0xFF);
			fputc(m,pf);
		};
		fclose(pf);
	};*/
	printf("\n");
	return 0;
}
/*--------------------------------------------------------------------*/
