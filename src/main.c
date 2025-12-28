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
 * DEMO 2: POLIFONIA 3 VOCES - SECUENCIA ÉPICA
 * ============================================================================ */

/* Acorde con sweep de filtro */
void poly3_filter(uint16_t n1, uint16_t n2, uint16_t n3, uint8_t up) {
    uint8_t fc;
    
    sid_freq(0, n1); sid_freq(1, n2); sid_freq(2, n3);
    sid_wave(0, SID_SAWTOOTH); sid_wave(1, SID_PULSE); sid_wave(2, SID_SAWTOOTH);
    sid_gate_on(0); sid_gate_on(1); sid_gate_on(2);
    
    sid_filter(0, 15, SID_FILT_V1 | SID_FILT_V2 | SID_FILT_V3, SID_FILT_LP);
    
    if (up) {
        for (fc = 20; fc < 100; fc += 4) {
            SID_FC_HI = fc;
            LEDS = ~(0x3F >> (fc >> 5));
            delay_ms(12);
        }
    } else {
        for (fc = 100; fc > 20; fc -= 4) {
            SID_FC_HI = fc;
            LEDS = ~(0x3F >> (fc >> 5));
            delay_ms(12);
        }
    }
    
    sid_gate_off(0); sid_gate_off(1); sid_gate_off(2);
    sid_filter_off();
    sid_volume(15);
    LEDS = 0xFF;
}

/* Bajo pulsante mientras suena acorde */
void bass_pulse(uint16_t bass, uint16_t n2, uint16_t n3, uint8_t pulses) {
    uint8_t i;
    
    sid_freq(1, n2); sid_freq(2, n3);
    sid_wave(1, SID_TRIANGLE); sid_wave(2, SID_PULSE);
    sid_gate_on(1); sid_gate_on(2);
    
    sid_freq(0, bass);
    sid_wave(0, SID_PULSE);
    
    for (i = 0; i < pulses; i++) {
        sid_gate_on(0);
        LEDS = ~(1 << (i % 6));
        delay_ms(80);
        sid_gate_off(0);
        delay_ms(80);
    }
    
    sid_gate_off(1); sid_gate_off(2);
    LEDS = 0xFF;
}

void poly3(uint16_t n1, uint16_t n2, uint16_t n3, uint16_t dur, uint8_t led) {
    LEDS = ~led;
    sid_freq(0, n1); sid_freq(1, n2); sid_freq(2, n3);
    sid_chord(n1, n2, n3);
    delay_ms(dur);
    sid_chord_off();
    LEDS = 0xFF;
}

/* ============================================================================
 * FUNCIONES DE BATERÍA (usando voz 3 con NOISE)
 * ============================================================================ */

/* Kick drum */
void drum_kick(void) {
    sid_freq(2, 0x0800);
    sid_adsr(2, ADSR_DRUM);
    sid_wave(2, SID_NOISE);
    sid_gate_on(2);
    LEDS = ~0x01;
    delay_ms(50);
    sid_gate_off(2);
}

/* Snare drum */
void drum_snare(void) {
    sid_freq(2, 0x2800);
    sid_adsr(2, ADSR_SNARE);
    sid_wave(2, SID_NOISE);
    sid_gate_on(2);
    LEDS = ~0x08;
    delay_ms(60);
    sid_gate_off(2);
}

/* Hi-hat */
void drum_hh(void) {
    sid_freq(2, 0x8000);
    sid_adsr(2, ADSR_DRUM);
    sid_wave(2, SID_NOISE);
    sid_gate_on(2);
    LEDS = ~0x20;
    delay_ms(25);
    sid_gate_off(2);
}

/* Batería con bajo y acorde */
void groove_bar(uint16_t bass, uint16_t chord, uint8_t bars) {
    uint8_t i;
    
    sid_adsr(0, ADSR_BASS);
    sid_pulse_width(0, 1024);
    sid_freq(0, bass);
    
    sid_adsr(1, ADSR_PAD);
    sid_pulse_width(1, 2048);
    sid_freq(1, chord);
    sid_wave(1, SID_PULSE);
    sid_gate_on(1);
    
    for (i = 0; i < bars; i++) {
        sid_wave(0, SID_PULSE);
        sid_gate_on(0);
        drum_kick();
        delay_ms(70);
        sid_gate_off(0);
        
        drum_hh();
        delay_ms(95);
        
        sid_gate_on(0);
        drum_snare();
        delay_ms(60);
        sid_gate_off(0);
        
        drum_hh();
        delay_ms(95);
    }
    
    sid_gate_off(1);
    LEDS = 0xFF;
}

void run_demo_polyphonic(void) {
    
    sid_init();
    sid_volume(15);
    
    /* Configurar voces */
    sid_voice(0, 0, SID_PULSE, ADSR_BASS);
    sid_pulse_width(0, 1024);
    
    sid_voice(1, 0, SID_PULSE, ADSR_PAD);
    sid_pulse_width(1, 2048);
    
    sid_voice(2, 0, SID_SAWTOOTH, ADSR_LEAD);
    
    uart_puts("\r\n[POLY]\r\n");
    
    /* === SECCIÓN 1: Acordes con sweep de filtro === */
    poly3_filter(NOTE_A3, NOTE_C4, NOTE_E4, 1);
    delay_ms(100);
    poly3_filter(NOTE_F3, NOTE_A3, NOTE_C4, 0);
    delay_ms(100);
    poly3_filter(NOTE_C3, NOTE_E3, NOTE_G3, 1);
    delay_ms(100);
    poly3_filter(NOTE_G3, NOTE_B3, NOTE_D4, 0);
    delay_ms(200);
    
    /* === SECCIÓN 2: Bajo pulsante con acordes suspendidos === */
    sid_adsr(0, ADSR_BASS);
    bass_pulse(NOTE_A2, NOTE_E4, NOTE_A4, 4);  /* Am */
    bass_pulse(NOTE_F2, NOTE_C4, NOTE_F4, 4);  /* F */
    
    /* === SECCIÓN 3: GROOVE CON BATERÍA === */
    uart_puts("[DRUMS]\r\n");
    sid_init();
    sid_volume(15);
    
    groove_bar(NOTE_A2, NOTE_E4, 2);
    groove_bar(NOTE_F2, NOTE_C4, 2);
    groove_bar(NOTE_C2, NOTE_G3, 2);
    groove_bar(NOTE_G2, NOTE_D4, 2);
    
    /* Fill final */
    drum_kick();
    delay_ms(80);
    drum_snare();
    delay_ms(80);
    drum_kick();
    delay_ms(80);
    drum_snare();
    delay_ms(200);
    
    /* === FINAL: Acorde mayor resolutivo === */
    sid_init();
    sid_volume(15);
    sid_voice(0, 0, SID_TRIANGLE, ADSR_STRINGS);
    sid_voice(1, 0, SID_PULSE, ADSR_STRINGS);
    sid_voice(2, 0, SID_SAWTOOTH, ADSR_STRINGS);
    sid_pulse_width(1, 2048);
    
    LEDS = 0x00;
    poly3(NOTE_A3, NOTE_C4, NOTE_E4, 400, 0x00);
    poly3(NOTE_A3, NOTE_CS4, NOTE_E4, 800, 0x00);
    LEDS = 0xFF;
    delay_ms(300);
}

/* ============================================================================
 * FILTER SWEEP CON LEDS
 * ============================================================================ */

void filter_sweep(void) {
    uint8_t fc;
    
    sid_freq(0, NOTE_A3);
    sid_adsr(0, ADSR_SFX);
    sid_wave(0, SID_SAWTOOTH);
    sid_gate_on(0);
    
    sid_filter(0, 15, SID_FILT_V1, SID_FILT_LP);
    
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
    
    sid_gate_off(0);
    sid_filter_off();
    sid_volume(15);
    LEDS = 0xFF;
}

/* ============================================================================
 * ESCALA MUSICAL CON LEDS
 * ============================================================================ */

void scale_c_major(void) {
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
        run_demo_polyphonic();
        delay_ms(1500);
    }
    return 0;
}
