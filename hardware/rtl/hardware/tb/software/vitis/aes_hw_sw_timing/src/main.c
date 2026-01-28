#include <stdio.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xtime_l.h"
#include "aes.h"

#define AES_BASEADDR   XPAR_MYIP_12_0_S00_AXI_BASEADDR

#define AES_REG_P0     0x00  // [127:96]
#define AES_REG_P1     0x04  // [95:64]
#define AES_REG_P2     0x08  // [63:32]
#define AES_REG_P3     0x0C  // [31:0]
#define AES_REG_CTRL   0x10  // bit0: start/ready

#define AES_WRITE(off, data)  Xil_Out32(AES_BASEADDR + (off), (data))
#define AES_READ(off)         Xil_In32 (AES_BASEADDR + (off))

//#define DEBUG_HW_ACCEL

static const uint8_t aes_key[16] = {
    0x00,0x01,0x02,0x03,
    0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,
    0x0c,0x0d,0x0e,0x0f
};

static const uint8_t aes_plaintext[16] = {
    0x00,0x11,0x22,0x33,
    0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb,
    0xcc,0xdd,0xee,0xff
};

// ---------- HW AES one block ----------
void aes_hw_encrypt_block(const uint8_t *pt, uint8_t *ct)
{
    u32 p0 = ((u32)pt[0]  << 24) | ((u32)pt[1] << 16) | ((u32)pt[2] << 8) | pt[3];
    u32 p1 = ((u32)pt[4]  << 24) | ((u32)pt[5] << 16) | ((u32)pt[6] << 8) | pt[7];
    u32 p2 = ((u32)pt[8]  << 24) | ((u32)pt[9] << 16) | ((u32)pt[10] << 8) | pt[11];
    u32 p3 = ((u32)pt[12] << 24) | ((u32)pt[13] << 16) | ((u32)pt[14] << 8) | pt[15];

    AES_WRITE(AES_REG_P0, p0);
    AES_WRITE(AES_REG_P1, p1);
    AES_WRITE(AES_REG_P2, p2);
    AES_WRITE(AES_REG_P3, p3);

    AES_WRITE(AES_REG_CTRL, 0x00000001);

    while ((AES_READ(AES_REG_CTRL) & 0x1) == 0) {
        ;
    }

    u32 c1 = AES_READ(AES_REG_P1);
    u32 c2 = AES_READ(AES_REG_P2);
    u32 c3 = AES_READ(AES_REG_P3);
    u32 c0 = AES_READ(AES_REG_P0);

//#ifdef DEBUG_HW_ACCEL
   // xil_printf("HW words: %08lx %08lx %08lx %08lx\r\n",
                   //(unsigned long)c0, (unsigned long)c1,
                  // (unsigned long)c2, (unsigned long)c3);
//#endif

    ct[0]  = (uint8_t)(c0 >> 24);
    ct[1]  = (uint8_t)(c0 >> 16);
    ct[2]  = (uint8_t)(c0 >>  8);
    ct[3]  = (uint8_t)(c0      );
    ct[4]  = (uint8_t)(c1 >> 24);
    ct[5]  = (uint8_t)(c1 >> 16);
    ct[6]  = (uint8_t)(c1 >>  8);
    ct[7]  = (uint8_t)(c1      );
    ct[8]  = (uint8_t)(c2 >> 24);
    ct[9]  = (uint8_t)(c2 >> 16);
    ct[10] = (uint8_t)(c2 >>  8);
    ct[11] = (uint8_t)(c2      );
    ct[12] = (uint8_t)(c3 >> 24);
    ct[13] = (uint8_t)(c3 >> 16);
    ct[14] = (uint8_t)(c3 >>  8);
    ct[15] = (uint8_t)(c3      );
}

// ---------- SW AES one block ----------
void aes_sw_encrypt_block(const uint8_t *pt, uint8_t *ct)
{
    struct AES_ctx ctx;
    uint8_t buf[16];

    for (int i = 0; i < 16; i++)
        buf[i] = pt[i];

    AES_init_ctx(&ctx, aes_key);
    AES_ECB_encrypt(&ctx, buf);

    for (int i = 0; i < 16; i++)
        ct[i] = buf[i];
}

// ---------- helpers ----------
void print_block(const char *label, const uint8_t *b)
{
    xil_printf("%s", label);
    for (int i = 0; i < 16; i++)
        xil_printf("%02x", b[i]);
    xil_printf("\r\n");
}

// ---------- main ----------
int main(void)
{
    XTime t1, t2;
    uint8_t ct_hw[16];
    uint8_t ct_sw[16];
    unsigned long long hw_ticks, sw_ticks;
    double hw_us, sw_us;

    xil_printf("AES HW/SW Timing Test\r\n");

    // HW AES
    XTime_GetTime(&t1);
    aes_hw_encrypt_block(aes_plaintext, ct_hw);
    XTime_GetTime(&t2);
    hw_ticks = (unsigned long long)(t2 - t1);
    hw_us = (double)hw_ticks / (COUNTS_PER_SECOND / 1000000.0);

    // SW AES
    XTime_GetTime(&t1);
    aes_sw_encrypt_block(aes_plaintext, ct_sw);
    XTime_GetTime(&t2);
    sw_ticks = (unsigned long long)(t2 - t1);
    sw_us = (double)sw_ticks / (COUNTS_PER_SECOND / 1000000.0);

    // Pack SW bytes into words for fair compare
    u32 sw0 = ((u32)ct_sw[0]  << 24) | ((u32)ct_sw[1] << 16) | ((u32)ct_sw[2] << 8) | ct_sw[3];
    u32 sw1 = ((u32)ct_sw[4]  << 24) | ((u32)ct_sw[5] << 16) | ((u32)ct_sw[6] << 8) | ct_sw[7];
    u32 sw2 = ((u32)ct_sw[8]  << 24) | ((u32)ct_sw[9] << 16) | ((u32)ct_sw[10] << 8) | ct_sw[11];
    u32 sw3 = ((u32)ct_sw[12] << 24) | ((u32)ct_sw[13] << 16) | ((u32)ct_sw[14] << 8) | ct_sw[15];

    // Read HW words again for comparison
    u32 c0 = AES_READ(AES_REG_P0);
    u32 c1 = AES_READ(AES_REG_P1);
    u32 c2 = AES_READ(AES_REG_P2);
    u32 c3 = AES_READ(AES_REG_P3);

    int match_words = (c0 == sw0) && (c1 == sw1) && (c2 == sw2) && (c3 == sw3);

    print_block("Plaintext : ", aes_plaintext);
    print_block("HW cipher : ", ct_hw);
    print_block("SW cipher : ", ct_sw);

    xil_printf("HW words: %08lx %08lx %08lx %08lx\r\n",
               (unsigned long)c0, (unsigned long)c1,
               (unsigned long)c2, (unsigned long)c3);
    xil_printf("SW words: %08lx %08lx %08lx %08lx\r\n",
               (unsigned long)sw0, (unsigned long)sw1,
               (unsigned long)sw2, (unsigned long)sw3);

    xil_printf("MATCH(words): %s\r\n", match_words ? "YES" : "NO");

    printf("HW: %llu ticks, %.3f us\r\n", hw_ticks, hw_us);
    printf("SW: %llu ticks, %.3f us\r\n", sw_ticks, sw_us);
    if (hw_us > 0.0)
        printf("Speedup (SW/HW): %.2fx\r\n", sw_us / hw_us);

    while (1) ;
    return 0;
}


