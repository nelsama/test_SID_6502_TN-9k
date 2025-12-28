/**
 * main.c - SID 6581 Spectacular Demo para Tang Nano 9K
 * 
 * Demo con LEDs sincronizados al ritmo del sonido
 * El ciclo completo se repite infinitamente
 * 
 * SID mapeado en $D400-$D41C (direcciones originales C64)
 */

#include <stdint.h>
#include "timer.h"
#include "uart.h"
#include "sid.h"

/* ============================================================================
 * REGISTROS DE HARDWARE (NO-SID)
 * ============================================================================ */

#define LEDS                 (*(volatile uint8_t*)0xC001)
#define CONF_PORT_SALIDA_LED (*(volatile uint8_t*)0xC003)

/* Aliases cortos para control bits (compatibilidad con código existente) */
#define GATE        SID_GATE
#define SYNC        SID_SYNC
#define RING        SID_RING
#define TEST        SID_TEST
#define TRIANGLE    SID_TRIANGLE
#define SAWTOOTH    SID_SAWTOOTH
#define PULSE       SID_PULSE
#define NOISE       SID_NOISE

/* Aliases cortos para modos de filtro */
#define FILT_LP     SID_FILT_LP
#define FILT_BP     SID_FILT_BP
#define FILT_HP     SID_FILT_HP

/* Las notas se toman de sid.h (librería SID) */

/* ============================================================================
 * FUNCIONES AUXILIARES
 * ============================================================================ */

void sid_clear(void) {
    sid_init();  /* Usar función de la librería */
}

/* Tocar nota CON LED sincronizado */
void play_note(uint16_t freq, uint8_t wave, uint16_t duration_ms, uint8_t led) {
    LEDS = ~led;  /* LED ON (activo bajo) */
    SID_V1_FREQ_LO = freq & 0xFF;
    SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
    SID_V1_CTRL = wave | GATE;
    delay_ms(duration_ms);
    SID_V1_CTRL = wave;
    LEDS = 0xFF;  /* LED OFF */
}

/* Tocar nota sin pausa al final */
void note_on(uint16_t freq, uint8_t wave, uint8_t led) {
    LEDS = ~led;
    SID_V1_FREQ_LO = freq & 0xFF;
    SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
    SID_V1_CTRL = wave | GATE;
}

void note_off(uint8_t wave) {
    SID_V1_CTRL = wave;
    LEDS = 0xFF;
}

/* ============================================================================
 * EFECTOS DE SONIDO CON LEDS
 * ============================================================================ */

void fx_laser(void) {
    uint16_t freq;
    uint8_t led_shift;
    
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xF0;
    
    led_shift = 0;
    for (freq = 0x3000; freq > 0x0200; freq -= 0x0100) {
        /* LEDs hacen efecto "cayendo" */
        LEDS = ~(0x80 >> (led_shift & 0x07));
        led_shift++;
        
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        SID_V1_CTRL = SAWTOOTH | GATE;
        delay_ms(8);
    }
    SID_V1_CTRL = 0;
    LEDS = 0xFF;
}

void fx_explosion(void) {
    uint8_t vol;
    uint16_t freq;
    
    SID_V1_AD = 0x00;  /* Attack=0, Decay=0 - sonido inmediato */
    SID_V1_SR = 0xF0;  /* Sustain=15, Release=0 */
    SID_MODE_VOL = 0x0F;
    LEDS = 0x00;
    
    /* Efecto de "boom" con NOISE - frecuencia alta bajando 
     * IMPORTANTE: NOISE necesita frecuencia > 0 para que el LFSR avance */
    SID_V1_FREQ_LO = 0xFF;
    SID_V1_FREQ_HI = 0xFF;  /* Frecuencia máxima para ruido denso */
    SID_V1_CTRL = NOISE | GATE;
    delay_ms(50);  /* Impacto inicial */
    
    /* Bajar frecuencia del ruido para simular "rumble" */
    for (freq = 0x8000; freq > 0x0800; freq -= 0x0400) {
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        delay_ms(8);
    }
    
    /* Fade out con NOISE - mantener frecuencia activa! */
    for (vol = 15; vol > 0; vol--) {
        SID_MODE_VOL = vol;
        
        /* LEDs van apagándose */
        if (vol > 12) LEDS = 0x00;
        else if (vol > 9) LEDS = 0x81;
        else if (vol > 6) LEDS = 0xC3;
        else if (vol > 3) LEDS = 0xE7;
        else LEDS = 0xFF;
        
        /* Frecuencia baja pero NO cero - el LFSR necesita que el acumulador cambie */
        SID_V1_FREQ_LO = 0x00;
        SID_V1_FREQ_HI = 0x08 + vol;  /* Mantener freq > 0 siempre */
        delay_ms(40);
    }
    
    SID_V1_CTRL = 0;
    SID_MODE_VOL = 0x0F;
    LEDS = 0xFF;
}

void fx_siren(uint8_t cycles) {
    uint8_t i;
    uint16_t freq;
    uint8_t led_pos;
    
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xF0;
    SID_V1_CTRL = TRIANGLE | GATE;
    
    for (i = 0; i < cycles; i++) {
        /* Subir - LEDs van de izquierda a derecha */
        led_pos = 0;
        for (freq = 0x1000; freq < 0x2000; freq += 0x80) {
            LEDS = ~(0x01 << (led_pos & 0x07));
            led_pos++;
            SID_V1_FREQ_LO = freq & 0xFF;
            SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
            delay_ms(6);
        }
        /* Bajar - LEDs van de derecha a izquierda */
        led_pos = 7;
        for (freq = 0x2000; freq > 0x1000; freq -= 0x80) {
            LEDS = ~(0x01 << (led_pos & 0x07));
            if (led_pos > 0) led_pos--;
            else led_pos = 7;
            SID_V1_FREQ_LO = freq & 0xFF;
            SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
            delay_ms(6);
        }
    }
    SID_V1_CTRL = 0;
    LEDS = 0xFF;
}

void fx_powerup(void) {
    uint16_t freq;
    uint8_t led_idx;
    
    SID_V1_AD = 0x09;
    SID_V1_SR = 0x00;
    SID_V1_PW_LO = 0x00;
    SID_V1_PW_HI = 0x08;
    
    led_idx = 0;
    for (freq = 0x0800; freq < 0x4000; freq += 0x0180) {
        /* LEDs se van encendiendo de abajo a arriba */
        LEDS = ~((1 << (led_idx + 1)) - 1);
        if (led_idx < 7) led_idx++;
        
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        SID_V1_CTRL = PULSE | GATE;
        delay_ms(25);
        SID_V1_CTRL = PULSE;
        delay_ms(10);
    }
    
    /* Flash final */
    LEDS = 0x00; delay_ms(50);
    LEDS = 0xFF; delay_ms(50);
    LEDS = 0x00; delay_ms(50);
    LEDS = 0xFF;
    SID_V1_CTRL = 0;
}

void fx_coin(void) {
    SID_V1_AD = 0x00;
    SID_V1_SR = 0x90;
    SID_V1_PW_LO = 0x00;
    SID_V1_PW_HI = 0x08;
    
    /* Nota alta + LED flash */
    LEDS = 0x00;
    SID_V1_FREQ_LO = NOTE_E6 & 0xFF;
    SID_V1_FREQ_HI = (NOTE_E6 >> 8) & 0xFF;
    SID_V1_CTRL = PULSE | GATE;
    delay_ms(80);
    
    LEDS = 0xFF;
    delay_ms(20);
    
    /* Nota más alta + otro flash */
    LEDS = 0x00;
    SID_V1_FREQ_LO = NOTE_G6 & 0xFF;
    SID_V1_FREQ_HI = (NOTE_G6 >> 8) & 0xFF;
    delay_ms(200);
    
    SID_V1_CTRL = 0;
    LEDS = 0xFF;
}

/* ============================================================================
 * ARPEGIOS CON LEDS
 * ============================================================================ */

void arpeggio_major(uint16_t root, uint8_t octaves) {
    uint8_t i;
    uint16_t freq;
    uint8_t led_base;
    
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xA0;
    
    for (i = 0; i < octaves; i++) {
        freq = root << i;
        led_base = (i * 3) & 0x07;
        
        /* Raíz */
        LEDS = ~(1 << led_base);
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        SID_V1_CTRL = SAWTOOTH | GATE;
        delay_ms(80);
        SID_V1_CTRL = SAWTOOTH;
        
        /* Tercera mayor */
        LEDS = ~(1 << ((led_base + 1) & 0x07));
        freq = (freq * 5) >> 2;
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        SID_V1_CTRL = SAWTOOTH | GATE;
        delay_ms(80);
        SID_V1_CTRL = SAWTOOTH;
        
        /* Quinta */
        LEDS = ~(1 << ((led_base + 2) & 0x07));
        freq = (root << i);
        freq = (freq * 3) >> 1;
        SID_V1_FREQ_LO = freq & 0xFF;
        SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
        SID_V1_CTRL = SAWTOOTH | GATE;
        delay_ms(80);
        SID_V1_CTRL = SAWTOOTH;
    }
    LEDS = 0xFF;
}

/* ============================================================================
 * PWM SWEEP CON LEDS
 * ============================================================================ */

void pwm_sweep(uint16_t freq, uint16_t duration_ms) {
    uint8_t pw;
    uint16_t step_ms;
    
    SID_V1_FREQ_LO = freq & 0xFF;
    SID_V1_FREQ_HI = (freq >> 8) & 0xFF;
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xF0;
    SID_V1_CTRL = PULSE | GATE;
    
    step_ms = duration_ms / 32;
    
    /* Sweep hacia arriba - LEDs de izq a der */
    for (pw = 1; pw < 15; pw++) {
        LEDS = ~(1 << (pw >> 1));
        SID_V1_PW_HI = pw;
        delay_ms(step_ms);
    }
    /* Sweep hacia abajo - LEDs de der a izq */
    for (pw = 15; pw > 1; pw--) {
        LEDS = ~(1 << (pw >> 1));
        SID_V1_PW_HI = pw;
        delay_ms(step_ms);
    }
    
    SID_V1_CTRL = 0;
    LEDS = 0xFF;
}

/* ============================================================================
 * MELODÍAS CON LEDS
 * ============================================================================ */

void melody_intro(void) {
    SID_V1_AD = 0x22;
    SID_V1_SR = 0xA2;
    
    /* Acorde ascendente con LEDs encendiéndose */
    play_note(NOTE_C4, SAWTOOTH, 150, 0x01);
    delay_ms(30);
    play_note(NOTE_E4, SAWTOOTH, 150, 0x03);
    delay_ms(30);
    play_note(NOTE_G4, SAWTOOTH, 150, 0x07);
    delay_ms(30);
    play_note(NOTE_C5, SAWTOOTH, 400, 0xFF);  /* Todos ON */
    delay_ms(100);
    
    /* Bajada con LEDs apagándose */
    play_note(NOTE_B4, SAWTOOTH, 100, 0x0F);
    play_note(NOTE_A4, SAWTOOTH, 100, 0x07);
    play_note(NOTE_G4, SAWTOOTH, 100, 0x03);
    play_note(NOTE_E4, SAWTOOTH, 300, 0x01);
}

void melody_game(void) {
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xA0;
    SID_V1_PW_LO = 0x00;
    SID_V1_PW_HI = 0x08;
    
    /* Tema tipo Mario con LEDs al ritmo */
    play_note(NOTE_E4, PULSE, 120, 0x81);
    play_note(NOTE_E4, PULSE, 120, 0x42);
    delay_ms(120);
    play_note(NOTE_E4, PULSE, 120, 0x24);
    delay_ms(120);
    play_note(NOTE_C4, PULSE, 120, 0x18);
    play_note(NOTE_E4, PULSE, 240, 0x3C);
    play_note(NOTE_G4, PULSE, 360, 0xFF);
    delay_ms(240);
    play_note(NOTE_G3, PULSE, 360, 0x18);
}

void melody_victory(void) {
    SID_V1_AD = 0x11;
    SID_V1_SR = 0xA1;
    
    play_note(NOTE_G4, TRIANGLE, 150, 0x03);
    play_note(NOTE_C5, TRIANGLE, 150, 0x0F);
    play_note(NOTE_E5, TRIANGLE, 150, 0x3F);
    play_note(NOTE_G5, TRIANGLE, 300, 0xFF);
    play_note(NOTE_E5, TRIANGLE, 150, 0x3F);
    play_note(NOTE_G5, TRIANGLE, 600, 0xFF);
    
    /* Flash de victoria! */
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF; delay_ms(100);
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF; delay_ms(100);
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF;
}

/* ============================================================================
 * DEMO 2: POLIFONIA 3 VOCES
 * ============================================================================ */

void poly3(uint16_t n1, uint16_t n2, uint16_t n3, uint16_t dur, uint8_t led) {
    LEDS = ~led;
    SID_V1_FREQ_LO = n1 & 0xFF; SID_V1_FREQ_HI = n1 >> 8;
    SID_V2_FREQ_LO = n2 & 0xFF; SID_V2_FREQ_HI = n2 >> 8;
    SID_V3_FREQ_LO = n3 & 0xFF; SID_V3_FREQ_HI = n3 >> 8;
    SID_V1_CTRL = TRIANGLE | GATE;
    SID_V2_CTRL = PULSE | GATE;
    SID_V3_CTRL = SAWTOOTH | GATE;
    delay_ms(dur);
    SID_V1_CTRL = TRIANGLE;
    SID_V2_CTRL = PULSE;
    SID_V3_CTRL = SAWTOOTH;
    LEDS = 0xFF;
}

void run_demo_polyphonic(void) {
    uint8_t i;
    
    sid_clear();
    SID_MODE_VOL = 0x0F;
    
    /* Configurar Voz 1 */
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xF0;
    
    /* Configurar Voz 2 */
    SID_V2_AD = 0x00;
    SID_V2_SR = 0xF0;
    SID_V2_PW_LO = 0x00;
    SID_V2_PW_HI = 0x08;
    
    /* Configurar Voz 3 */
    SID_V3_AD = 0x00;
    SID_V3_SR = 0xF0;
    
    uart_puts("\r\n[POLY]\r\n");
    
    /* Canon x2 */
    for (i = 0; i < 2; i++) {
        poly3(NOTE_D4, NOTE_A4, NOTE_D3, 300, 0x03);
        poly3(NOTE_E4, NOTE_A4, NOTE_A2, 300, 0x0C);
        poly3(NOTE_D4, NOTE_B4, NOTE_B2, 300, 0x30);
        poly3(NOTE_B3, NOTE_D4, NOTE_G2, 300, 0xC0);
    }
    
    /* C-F-G-C */
    poly3(NOTE_C4, NOTE_E4, NOTE_C2, 250, 0x0F);
    poly3(NOTE_F4, NOTE_A4, NOTE_F2, 250, 0xF0);
    poly3(NOTE_G4, NOTE_B4, NOTE_G2, 250, 0x0F);
    poly3(NOTE_C4, NOTE_E4, NOTE_C2, 500, 0xFF);
}

/* ============================================================================
 * FILTER SWEEP CON LEDS
 * ============================================================================ */

void filter_sweep(void) {
    uint8_t fc;
    
    SID_V1_FREQ_LO = NOTE_A3 & 0xFF;
    SID_V1_FREQ_HI = (NOTE_A3 >> 8) & 0xFF;
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xF0;
    SID_V1_CTRL = SAWTOOTH | GATE;
    
    SID_RES_FILT = 0xF1;
    SID_MODE_VOL = FILT_LP | 0x0F;
    
    /* Sweep del filtro con LEDs siguiendo */
    for (fc = 10; fc < 120; fc += 2) {
        LEDS = ~(1 << ((fc >> 4) & 0x07));
        SID_FC_HI = fc;
        delay_ms(15);
    }
    for (fc = 120; fc > 10; fc -= 2) {
        LEDS = ~(1 << ((fc >> 4) & 0x07));
        SID_FC_HI = fc;
        delay_ms(15);
    }
    
    SID_V1_CTRL = 0;
    SID_RES_FILT = 0x00;
    SID_MODE_VOL = 0x0F;
    LEDS = 0xFF;
}

/* ============================================================================
 * ESCALA MUSICAL CON LEDS
 * ============================================================================ */

void scale_c_major(void) {
    SID_V1_AD = 0x00;
    SID_V1_SR = 0xB0;
    
    /* C4 - C5 con LED diferente para cada nota */
    play_note(NOTE_C4, SAWTOOTH, 200, 0x01);
    play_note(NOTE_D4, SAWTOOTH, 200, 0x02);
    play_note(NOTE_E4, SAWTOOTH, 200, 0x04);
    play_note(NOTE_F4, SAWTOOTH, 200, 0x08);
    play_note(NOTE_G4, SAWTOOTH, 200, 0x10);
    play_note(NOTE_A4, SAWTOOTH, 200, 0x20);
    play_note(NOTE_B4, SAWTOOTH, 200, 0x40);
    play_note(NOTE_C5, SAWTOOTH, 400, 0x80);
    
    /* Bajar */
    play_note(NOTE_B4, SAWTOOTH, 200, 0x40);
    play_note(NOTE_A4, SAWTOOTH, 200, 0x20);
    play_note(NOTE_G4, SAWTOOTH, 200, 0x10);
    play_note(NOTE_F4, SAWTOOTH, 200, 0x08);
    play_note(NOTE_E4, SAWTOOTH, 200, 0x04);
    play_note(NOTE_D4, SAWTOOTH, 200, 0x02);
    play_note(NOTE_C4, SAWTOOTH, 400, 0x01);
}

/* ============================================================================
 * SECUENCIA COMPLETA DEL DEMO
 * ============================================================================ */

void run_demo(void) {
    sid_clear();
    SID_MODE_VOL = 0x0F;
    
    uart_puts("\r\n[DEMO 1]\r\n");
    
    melody_intro();
    delay_ms(400);
    
    scale_c_major();
    delay_ms(400);
    
    fx_laser();
    delay_ms(200);
    
    fx_explosion();
    delay_ms(200);
    
    fx_coin();
    delay_ms(200);
    
    fx_powerup();
    delay_ms(200);
    
    fx_siren(2);
    delay_ms(300);
    
    arpeggio_major(NOTE_C4, 3);
    delay_ms(200);
    arpeggio_major(NOTE_G3, 3);
    delay_ms(300);
    
    pwm_sweep(NOTE_A4, 2000);
    delay_ms(300);
    
    filter_sweep();
    delay_ms(300);
    
    melody_game();
    delay_ms(300);
    
    melody_victory();
    delay_ms(500);
}

/* ============================================================================
 * PROGRAMA PRINCIPAL
 * ============================================================================ */

int main(void) {
    CONF_PORT_SALIDA_LED = 0xC0;
    LEDS = 0xFF;
    
    uart_init();
    timer_init();
    
    uart_puts("\r\n=== SID DEMO ===\r\n");
    
    while (1) {
        run_demo();
        delay_ms(1000);
        run_demo_polyphonic();
        delay_ms(1500);
    }
    return 0;
}
