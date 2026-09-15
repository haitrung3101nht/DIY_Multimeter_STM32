"""Host checks for bounded text transfers, coalescing and font coverage."""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
    p=Path(directory)
    (p/'main.h').write_text('''#include <stdint.h>
#define TFT_DC_GPIO_Port 0
#define TFT_DC_Pin 0
#define TFT_RST_GPIO_Port 0
#define TFT_RST_Pin 1
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define HAL_OK 0
void HAL_GPIO_WritePin(int,int,int);
void HAL_Delay(unsigned);
void Error_Handler(void);
''')
    (p/'spi.h').write_text('''extern int hspi1;
int HAL_SPI_Transmit(int*, unsigned char*, unsigned short, unsigned);
''')
    (p/'test.c').write_text(r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tft.h"
static unsigned rows, dc, cmd, x0,x1,y0,y1;
static uint16_t display[240][240];
int hspi1;
void HAL_GPIO_WritePin(int a,int b,int c){(void)a;if(b==0) dc=(unsigned)c;}
void HAL_Delay(unsigned t){(void)t;}
void Error_Handler(void){abort();}
int HAL_SPI_Transmit(int *spi,unsigned char *p,unsigned short n,unsigned t){
    (void)spi;(void)t; assert(n<=480);
    if(!dc){assert(n==1);cmd=p[0];}
    else if(cmd==0x2A){assert(n==4);x0=(p[0]<<8)|p[1];x1=(p[2]<<8)|p[3];}
    else if(cmd==0x2B){assert(n==4);y0=(p[0]<<8)|p[1];y1=(p[2]<<8)|p[3];}
    else if(cmd==0x2C){assert(x0==0 && x1==239 && y0==y1 && y0<240 && n==480);
        rows++;for(unsigned x=0;x<240;x++)display[y0][x]=(uint16_t)((p[x*2]<<8)|p[x*2+1]);}
    return 0;
}
static void drain(void){for(unsigned i=0;i<240;i++){unsigned before=rows;TFT_Process();assert(rows<=before+1);}}
static unsigned lit(unsigned x0_,unsigned y0_,unsigned x1_,unsigned y1_){
    unsigned n=0;for(unsigned y=y0_;y<y1_;y++)for(unsigned x=x0_;x<x1_;x++)n+=display[y][x]!=0;return n;
}
int main(void){
    TFT_SetText(0,10,10,"A",TFT_WHITE);drain(); assert(lit(10,10,26,38)>0);
    unsigned before=rows;TFT_SetText(0,10,10,"A",TFT_WHITE);drain();assert(rows==before);
    TFT_SetText(1,100,10,"B",TFT_GREEN);drain();unsigned other=lit(100,10,116,38);
    TFT_SetText(0,40,60,"A",TFT_WHITE);drain();assert(!lit(10,10,26,38));assert(lit(40,60,56,88)>0);
    assert(lit(100,10,116,38)==other); /* Shared scanline content preserved. */
    TFT_SetText(0,40,60,"",TFT_WHITE);drain();assert(!lit(40,60,56,88));
    TFT_SetText(2,239,230,"AB",TFT_WHITE);drain(); /* Both edges clipped. */
    TFT_RemoveText(1);drain();assert(!lit(100,10,116,38));
    TFT_SetText(3,20,100,"OLD",TFT_WHITE);TFT_Process();
    TFT_ClearScene();TFT_SetText(0,80,40,"NEW",TFT_WHITE);drain();
    assert(!lit(0,100,240,240));assert(lit(80,40,128,68)>0);
    before=rows;TFT_SetText(16,0,0,"bad",TFT_WHITE);TFT_SetText(0,240,0,"bad",TFT_WHITE);drain();assert(rows==before);
    puts("PASS: XY move, erase, shared row, edge clipping, scene switch, bounded transfers");
}

''')
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',f'-I{p}',f'-I{root/"Modules/TFT/Inc"}',str(root/'Modules/TFT/Src/tft.c'),str(p/'test.c'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
