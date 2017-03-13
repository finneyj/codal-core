#ifndef DEVICE_NEOPIXEL_H
#define DEVICE_NEOPIXEL_H

// The timings are taken from Adafruit's NeoPixel library

#include "DevicePin.h"

static void neopixel_send_buffer_core(volatile uint32_t *clraddr, uint32_t pinMask,
                                      const uint8_t *ptr, int numBytes) __attribute__((naked));

static void neopixel_send_buffer_core(volatile uint32_t *clraddr, uint32_t pinMask,
                                      const uint8_t *ptr, int numBytes)
{
    asm volatile("        push    {r4, r5, r6, lr};"
                 "        add     r3, r2, r3;"
                 "        cpsid   i;"
                 "loopLoad:"
                 "        ldrb r5, [r2, #0];" // r5 := *ptr
                 "        add  r2, #1;"       // ptr++
                 "        movs    r4, #128;"  // r4-mask, 0x80
                 "loopBit:"
                 "        str r1, [r0, #4];"                    // set
                 "        movs r6, #3; d2: sub r6, #1; bne d2;" // delay 3
                 "        tst r4, r5;"                          // mask&r5
                 "        bne skipclr;"
                 "        str r1, [r0, #0];" // clr
                 "skipclr:"
                 "        movs r6, #6; d0: sub r6, #1; bne d0;" // delay 6
                 "        str r1, [r0, #0];"   // clr (possibly again, doesn't matter)
                 "        asr     r4, r4, #1;" // mask >>= 1
                 "        beq     nextbyte;"
                 "        uxtb    r4, r4;"
                 "        movs r6, #2; d1: sub r6, #1; bne d1;" // delay 2
                 "        b       loopBit;"
                 "nextbyte:"
                 "        cmp r2, r3;"
                 "        bcs stop;"
                 "        b loopLoad;"
                 "stop:"
                 "        cpsie i;"
                 "        pop {r4, r5, r6, pc};"
                 "");
}

// this assumes the pin has been configured correctly
static inline void neopixel_send_buffer(DevicePin &pin, const uint8_t *ptr, int numBytes)
{
    int pinid = (int)pin.name;

    uint8_t portNum = pinid / 32;
    uint32_t pinMask = 1ul << (pinid % 32);

    volatile uint32_t *clraddr = &PORT->Group[portNum].OUTCLR.reg;

    pin.setDigitalValue(0); // make sure it's configured as digital out
    neopixel_send_buffer_core(clraddr, pinMask, ptr, numBytes);
    pin.setDigitalValue(0);
}
#endif
