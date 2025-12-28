/**
 * main.c - SID 6581 Spectacular Demo para Tang Nano 9K
 * 
 * Demo con LEDs sincronizados al ritmo del sonido
 * El ciclo completo se repite infinitamente
 * Usa librería SID optimizada en ASM
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

/* ============================================================================
 * FUNCIONES AUXILIARES
 * ============================================================================ */

/* Tocar nota CON LED sincronizado */
void play_note(uint16_t freq, uint8_t wave, uint16_t duration_ms, uint8_t led) {
    LEDS = ~led;
    sid_adsr(0, ADSR_LEAD);
    sid_freq(0, freq);
    sid_wave(0, wave);
    sid_gate_on(0);
    delay_ms(duration_ms);
    sid_gate_off(0);
    LEDS = 0xFF;
}

/* ============================================================================
 * EFECTOS DE SONIDO CON LEDS
 * ============================================================================ */

void fx_laser(void) {
    uint16_t freq;
    uint8_t led_shift;
    
    uart_puts("  Laser\r\n");
    sid_adsr(0, ADSR_SFX);
    
    led_shift = 0;
    for (freq = 0x3000; freq > 0x0200; freq -= 0x0100) {
        LEDS = ~(0x80 >> (led_shift & 0x07));
        led_shift++;
        sid_freq(0, freq);
        sid_wave(0, SID_SAWTOOTH);
        sid_gate_on(0);
        delay_ms(8);
    }
    sid_gate_off(0);
    LEDS = 0xFF;
}

void fx_explosion(void) {
    uint8_t vol;
    uint16_t freq;
    
    uart_puts("  Explosion\r\n");
    sid_adsr(0, ADSR_SFX);
    sid_volume(15);
    LEDS = 0x00;
    
    sid_freq(0, 0xFFFF);
    sid_wave(0, SID_NOISE);
    sid_gate_on(0);
    delay_ms(50);
    
    for (freq = 0x8000; freq > 0x0800; freq -= 0x0400) {
        sid_freq(0, freq);
        delay_ms(8);
    }
    
    for (vol = 15; vol > 0; vol--) {
        sid_volume(vol);
        if (vol > 12) LEDS = 0x00;
        else if (vol > 9) LEDS = 0x81;
        else if (vol > 6) LEDS = 0xC3;
        else if (vol > 3) LEDS = 0xE7;
        else LEDS = 0xFF;
        sid_freq(0, 0x0800 + (vol << 8));
        delay_ms(40);
    }
    
    sid_gate_off(0);
    sid_volume(15);
    LEDS = 0xFF;
}

void fx_siren(uint8_t cycles) {
    uint8_t i;
    uint16_t freq;
    uint8_t led_pos;
    
    uart_puts("  Siren\r\n");
    sid_adsr(0, ADSR_SFX);
    sid_wave(0, SID_TRIANGLE);
    sid_gate_on(0);
    
    for (i = 0; i < cycles; i++) {
        led_pos = 0;
        for (freq = 0x1000; freq < 0x2000; freq += 0x80) {
            LEDS = ~(0x01 << (led_pos & 0x07));
            led_pos++;
            sid_freq(0, freq);
            delay_ms(6);
        }
        led_pos = 7;
        for (freq = 0x2000; freq > 0x1000; freq -= 0x80) {
            LEDS = ~(0x01 << (led_pos & 0x07));
            if (led_pos > 0) led_pos--;
            else led_pos = 7;
            sid_freq(0, freq);
            delay_ms(6);
        }
    }
    sid_gate_off(0);
    LEDS = 0xFF;
}

void fx_powerup(void) {
    uint16_t freq;
    uint8_t led_idx;
    
    uart_puts("  PowerUp\r\n");
    sid_adsr(0, ADSR_PLUCK);
    sid_pulse_width(0, 2048);
    
    led_idx = 0;
    for (freq = 0x0800; freq < 0x4000; freq += 0x0180) {
        LEDS = ~((1 << (led_idx + 1)) - 1);
        if (led_idx < 7) led_idx++;
        sid_freq(0, freq);
        sid_wave(0, SID_PULSE);
        sid_gate_on(0);
        delay_ms(25);
        sid_gate_off(0);
        delay_ms(10);
    }
    
    LEDS = 0x00; delay_ms(50);
    LEDS = 0xFF; delay_ms(50);
    LEDS = 0x00; delay_ms(50);
    LEDS = 0xFF;
    sid_gate_off(0);
}

void fx_coin(void) {
    uart_puts("  Coin\r\n");
    sid_adsr(0, ADSR_PLUCK);
    sid_pulse_width(0, 2048);
    
    LEDS = 0x00;
    sid_freq(0, NOTE_E6);
    sid_wave(0, SID_PULSE);
    sid_gate_on(0);
    delay_ms(80);
    
    LEDS = 0xFF;
    delay_ms(20);
    
    LEDS = 0x00;
    sid_freq(0, NOTE_G6);
    delay_ms(200);
    
    sid_gate_off(0);
    LEDS = 0xFF;
}

/* ============================================================================
 * ARPEGIOS CON LEDS
 * ============================================================================ */

void arpeggio_major(uint16_t root, uint8_t octaves) {
    uint8_t i;
    uint16_t freq;
    uint8_t led_base;
    
    uart_puts("  Arpeggio\r\n");
    sid_adsr(0, ADSR_FAST);
    
    for (i = 0; i < octaves; i++) {
        freq = root << i;
        led_base = (i * 3) & 0x07;
        
        /* Raíz */
        LEDS = ~(1 << led_base);
        sid_freq(0, freq);
        sid_wave(0, SID_SAWTOOTH);
        sid_gate_on(0);
        delay_ms(80);
        sid_gate_off(0);
        
        /* Tercera mayor */
        LEDS = ~(1 << ((led_base + 1) & 0x07));
        freq = (freq * 5) >> 2;
        sid_freq(0, freq);
        sid_gate_on(0);
        delay_ms(80);
        sid_gate_off(0);
        
        /* Quinta */
        LEDS = ~(1 << ((led_base + 2) & 0x07));
        freq = (root << i);
        freq = (freq * 3) >> 1;
        sid_freq(0, freq);
        sid_gate_on(0);
        delay_ms(80);
        sid_gate_off(0);
    }
    LEDS = 0xFF;
}

/* ============================================================================
 * PWM SWEEP CON LEDS
 * ============================================================================ */

void pwm_sweep(uint16_t freq, uint16_t duration_ms) {
    uint8_t pw;
    uint16_t step_ms;
    
    uart_puts("  PWM Sweep\r\n");
    sid_freq(0, freq);
    sid_adsr(0, ADSR_SFX);
    sid_wave(0, SID_PULSE);
    sid_gate_on(0);
    
    step_ms = duration_ms / 32;
    
    for (pw = 1; pw < 15; pw++) {
        LEDS = ~(1 << (pw >> 1));
        sid_pulse_width(0, pw << 8);
        delay_ms(step_ms);
    }
    for (pw = 15; pw > 1; pw--) {
        LEDS = ~(1 << (pw >> 1));
        sid_pulse_width(0, pw << 8);
        delay_ms(step_ms);
    }
    
    sid_gate_off(0);
    LEDS = 0xFF;
}

/* ============================================================================
 * MELODÍAS CON LEDS
 * ============================================================================ */

void melody_intro(void) {
    uart_puts("  Melody Intro\r\n");
    sid_adsr(0, ADSR_STRINGS);
    
    play_note(NOTE_C4, SID_SAWTOOTH, 150, 0x01);
    delay_ms(30);
    play_note(NOTE_E4, SID_SAWTOOTH, 150, 0x03);
    delay_ms(30);
    play_note(NOTE_G4, SID_SAWTOOTH, 150, 0x07);
    delay_ms(30);
    play_note(NOTE_C5, SID_SAWTOOTH, 400, 0xFF);
    delay_ms(100);
    
    play_note(NOTE_B4, SID_SAWTOOTH, 100, 0x0F);
    play_note(NOTE_A4, SID_SAWTOOTH, 100, 0x07);
    play_note(NOTE_G4, SID_SAWTOOTH, 100, 0x03);
    play_note(NOTE_E4, SID_SAWTOOTH, 300, 0x01);
}

void melody_game(void) {
    uart_puts("  Melody Game\r\n");
    sid_adsr(0, ADSR_FAST);
    sid_pulse_width(0, 2048);
    
    play_note(NOTE_E4, SID_PULSE, 120, 0x81);
    play_note(NOTE_E4, SID_PULSE, 120, 0x42);
    delay_ms(120);
    play_note(NOTE_E4, SID_PULSE, 120, 0x24);
    delay_ms(120);
    play_note(NOTE_C4, SID_PULSE, 120, 0x18);
    play_note(NOTE_E4, SID_PULSE, 240, 0x3C);
    play_note(NOTE_G4, SID_PULSE, 360, 0xFF);
    delay_ms(240);
    play_note(NOTE_G3, SID_PULSE, 360, 0x18);
}

void melody_victory(void) {
    uart_puts("  Victory!\r\n");
    sid_adsr(0, ADSR_LEAD);
    
    play_note(NOTE_G4, SID_TRIANGLE, 150, 0x03);
    play_note(NOTE_C5, SID_TRIANGLE, 150, 0x0F);
    play_note(NOTE_E5, SID_TRIANGLE, 150, 0x3F);
    play_note(NOTE_G5, SID_TRIANGLE, 300, 0xFF);
    play_note(NOTE_E5, SID_TRIANGLE, 150, 0x3F);
    play_note(NOTE_G5, SID_TRIANGLE, 600, 0xFF);
    
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF; delay_ms(100);
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF; delay_ms(100);
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF;
}

/* ============================================================================
 * SUPER MARIO BROS THEME - 3 VOCES
 * Voz 0: Bajo
 * Voz 1: Melodía
 * Voz 2: Percusión
 * ============================================================================ */

/* Nota de melodía SIN delay */
void mario_note(uint16_t freq) {
    if (freq == 0) {
        sid_gate_off(1);
    } else {
        sid_freq(1, freq);
        sid_gate_on(1);
    }
}

/* Beat de batería SIN delay */
void mario_beat(uint8_t type) {
    if (type == 0) {
        /* Kick */
        sid_freq(2, 0x0600);
    } else {
        /* Hi-hat */
        sid_freq(2, 0x4000);
    }
    sid_wave(2, SID_NOISE);
    sid_gate_on(2);
}

/* Bajo SIN delay */
void mario_bass(uint16_t freq) {
    if (freq == 0) {
        sid_gate_off(0);
    } else {
        sid_freq(0, freq);
        sid_gate_on(0);
    }
}

/* Tempo: duración de un beat en ms */
#define TEMPO 125

/* Intro de Mario - SINCRONIZADO */
void mario_intro(void) {
    sid_init();
    sid_volume(15);
    
    /* Configurar voces */
    sid_adsr(0, ADSR_BASS);
    sid_wave(0, SID_PULSE);
    sid_pulse_width(0, 1024);
    
    sid_adsr(1, ADSR_FAST);
    sid_wave(1, SID_PULSE);
    sid_pulse_width(1, 2048);
    
    sid_adsr(2, ADSR_DRUM);
    
    uart_puts("\r\n[MARIO BROS]\r\n");
    
    /* ===== Compás 1: E E _ E ===== */
    /* Beat 1: E */
    mario_bass(NOTE_E2); mario_beat(0); mario_note(NOTE_E5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* Beat 2: E */
    mario_beat(1); mario_note(NOTE_E5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* Beat 3: silencio */
    mario_beat(0);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* Beat 4: E */
    mario_beat(1); mario_note(NOTE_E5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* ===== Compás 2: _ C E _ ===== */
    /* Beat 1: silencio */
    mario_bass(NOTE_C2); mario_beat(0);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* Beat 2: C */
    mario_beat(1); mario_note(NOTE_C5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* Beat 3: E */
    mario_beat(0); mario_note(NOTE_E5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* Beat 4: silencio */
    mario_beat(1);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* ===== Compás 3: G _ _ _ ===== */
    mario_bass(NOTE_G2); mario_beat(0); mario_note(NOTE_G5);
    delay_ms(TEMPO * 2);
    sid_gate_off(1); sid_gate_off(2);
    mario_beat(0);
    delay_ms(TEMPO * 2);
    sid_gate_off(0); sid_gate_off(2);
    
    /* ===== Compás 4: G bajo ===== */
    mario_bass(NOTE_G1); mario_beat(0); mario_note(NOTE_G4);
    delay_ms(TEMPO * 2);
    sid_gate_off(1); sid_gate_off(2);
    delay_ms(TEMPO * 2);
    sid_gate_off(0);
    
    /* ===== Compás 5: C _ G _ ===== */
    mario_bass(NOTE_C3); mario_beat(0); mario_note(NOTE_C5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    mario_bass(NOTE_G2); mario_beat(0); mario_note(NOTE_G4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* ===== Compás 6: E _ A _ ===== */
    mario_bass(NOTE_E2); mario_beat(0); mario_note(NOTE_E4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    mario_bass(NOTE_A2); mario_beat(0); mario_note(NOTE_A4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1); mario_note(NOTE_B4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    /* ===== Compás 7: Bb A G ===== */
    mario_bass(NOTE_AS2); mario_beat(0); mario_note(NOTE_AS4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1); mario_note(NOTE_A4);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_bass(NOTE_G2); mario_beat(0); mario_note(NOTE_G4);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* G E G (triplet feel) */
    mario_note(NOTE_E5);
    delay_ms(TEMPO);
    
    mario_beat(0); mario_note(NOTE_G5);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* ===== Compás 8: A F G _ ===== */
    mario_bass(NOTE_F2); mario_beat(1); mario_note(NOTE_A5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(0); mario_note(NOTE_F5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(1); mario_note(NOTE_G5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(0);
    delay_ms(TEMPO);
    sid_gate_off(2);
    
    /* ===== Compás 9: E C D B ===== */
    mario_bass(NOTE_C3); mario_beat(1); mario_note(NOTE_E5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(0); mario_note(NOTE_C5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_bass(NOTE_G2); mario_beat(1); mario_note(NOTE_D5);
    delay_ms(TEMPO);
    sid_gate_off(1); sid_gate_off(2);
    
    mario_beat(0); mario_note(NOTE_B4);
    delay_ms(TEMPO * 2);
    
    /* Final */
    sid_gate_off(0);
    sid_gate_off(1);
    sid_gate_off(2);
    
    uart_puts("  Done!\r\n");
    
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF; delay_ms(100);
    LEDS = 0x00; delay_ms(100);
    LEDS = 0xFF;
}

/* ============================================================================
 * FILTER SWEEP CON LEDS
 * ============================================================================ */

void filter_sweep(void) {
    uint16_t cutoff;
    
    uart_puts("  Filter Sweep\r\n");
    sid_volume(15);
    sid_adsr(0, ADSR_SFX);
    sid_freq(0, NOTE_A3);
    sid_wave(0, SID_SAWTOOTH);
    
    /* Activar filtro LP en voz 0, resonancia alta */
    sid_filter(2047, 8, SID_FILT_V1, SID_FILT_LP);
    
    sid_gate_on(0);
    
    /* Sweep de cutoff bajando (de 2047 a 100) */
    for (cutoff = 2047; cutoff > 100; cutoff -= 32) {
        LEDS = ~(1 << ((cutoff >> 8) & 0x07));
        sid_filter_cutoff(cutoff);
        delay_ms(15);
    }
    /* Sweep de cutoff subiendo (de 100 a 2047) */
    for (cutoff = 100; cutoff < 2047; cutoff += 32) {
        LEDS = ~(1 << ((cutoff >> 8) & 0x07));
        sid_filter_cutoff(cutoff);
        delay_ms(15);
    }
    
    sid_gate_off(0);
    
    /* Workaround: el VHDL del NetSID no limpia estados internos */
    sid_filter_off();
    sid_volume(15);
    
    delay_ms(100);
    LEDS = 0xFF;
}

/* ============================================================================
 * ESCALA MUSICAL CON LEDS
 * ============================================================================ */

void scale_c_major(void) {
    uart_puts("  Scale C Major\r\n");
    sid_adsr(0, ADSR_LEAD);
    
    play_note(NOTE_C4, SID_SAWTOOTH, 200, 0x01);
    play_note(NOTE_D4, SID_SAWTOOTH, 200, 0x02);
    play_note(NOTE_E4, SID_SAWTOOTH, 200, 0x04);
    play_note(NOTE_F4, SID_SAWTOOTH, 200, 0x08);
    play_note(NOTE_G4, SID_SAWTOOTH, 200, 0x10);
    play_note(NOTE_A4, SID_SAWTOOTH, 200, 0x20);
    play_note(NOTE_B4, SID_SAWTOOTH, 200, 0x40);
    play_note(NOTE_C5, SID_SAWTOOTH, 400, 0x80);
    
    /* Bajar */
    play_note(NOTE_B4, SID_SAWTOOTH, 200, 0x40);
    play_note(NOTE_A4, SID_SAWTOOTH, 200, 0x20);
    play_note(NOTE_G4, SID_SAWTOOTH, 200, 0x10);
    play_note(NOTE_F4, SID_SAWTOOTH, 200, 0x08);
    play_note(NOTE_E4, SID_SAWTOOTH, 200, 0x04);
    play_note(NOTE_D4, SID_SAWTOOTH, 200, 0x02);
    play_note(NOTE_C4, SID_SAWTOOTH, 400, 0x01);
}

/* ============================================================================
 * SECUENCIA COMPLETA DEL DEMO
 * ============================================================================ */

void run_demo(void) {
    sid_init();
    sid_volume(15);
    
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
        mario_intro();
        delay_ms(1500);
    }
    return 0;
}
