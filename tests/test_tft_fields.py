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
static unsigned rows, partial_pixels;
int hspi1;
void HAL_GPIO_WritePin(int a,int b,int c){(void)a;(void)b;(void)c;}
void HAL_Delay(unsigned t){(void)t;}
void Error_Handler(void){abort();}
int HAL_SPI_Transmit(int *spi,unsigned char *p,unsigned short n,unsigned t){
    (void)spi;(void)t; assert(n<=416);
    if(n==416){rows++; for(unsigned i=0;i<n;i+=2){unsigned v=(p[i]<<8)|p[i+1]; if(v && v!=65535) partial_pixels++;}}
    return 0;
}
int main(void){
    TFT_SetText(0,8,"-11.5 ABCD*#",TFT_WHITE);
    for(unsigned i=0;i<28;i++){unsigned before=rows;TFT_Process();assert(rows==before+1);}
    assert(partial_pixels>0); /* Antialiased coverage, not just black/white. */
    TFT_SetText(0,8,"-11.5 ABCD*#",TFT_WHITE);
    TFT_Process();assert(rows==28); /* Unchanged content is not retransmitted. */
    TFT_SetText(0,8,"1",TFT_WHITE);TFT_Process();
    TFT_SetText(0,8,"2",TFT_WHITE); /* Latest update must survive an active transfer. */
    for(unsigned i=0;i<55;i++){TFT_Process();} assert(rows==84);
    TFT_SetText(6,8,"invalid",TFT_WHITE);TFT_SetText(1,239,"invalid",TFT_WHITE);
    TFT_Process();assert(rows==84);
    TFT_SetText(0,8,"OLD PAGE",TFT_WHITE); TFT_Process();
    TFT_InvalidateText();
    for(unsigned i=0;i<6;i++) TFT_SetText(i,8+32*i,"NEW",TFT_WHITE);
    unsigned before=rows;
    for(unsigned i=0;i<168;i++) TFT_Process();
    assert(rows==before+168); /* All six fields fully repainted after cancellation. */
    puts("PASS: bounded scanlines, antialiasing, unchanged fields, queued updates, clipping");
}
''')
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',f'-I{p}',f'-I{root/"Modules/TFT/Inc"}',str(root/'Modules/TFT/Src/tft.c'),str(p/'test.c'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
