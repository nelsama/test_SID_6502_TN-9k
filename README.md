# 🎵 SID 6581 Demo - Tang Nano 9K + 6502

🚀 **Demo musical del legendario chip SID 6581** del Commodore 64, implementado en FPGA Tang Nano 9K con CPU 6502.

Demo espectacular con efectos de sonido y LEDs sincronizados al ritmo de la música.

## 🎬 Características del Demo

- 🎵 **Efectos de sonido**: Láser, sirenas, explosiones, arpeggios
- 🎹 **Melodías**: Escalas, acordes, secuencias musicales
- 💡 **LEDs sincronizados** al ritmo del sonido
- 🔄 **Loop infinito** con transiciones suaves

## Hardware

- ✅ CPU 6502 @ 3.375 MHz en FPGA Tang Nano 9K
- ✅ **Chip de sonido SID 6581** (3 voces, filtros, ADSR)
- ✅ Control de 6 LEDs sincronizados
- ✅ Comunicación UART para debug
- ✅ Timer de precisión

## Mapa de Hardware

| Componente | Dirección | Descripción |
|------------|-----------|-------------|
| LEDs | $C001 | 6 LEDs (activo bajo) |
| CONF_PORT | $C003 | Configuración puerto LEDs |
| UART | $C020-$C021 | Comunicación serial 115200 baud |
| Timer | $C030-$C03C | Timer de precisión |
| **SID 6581** | $D400-$D41C | Chip de sonido (compatible C64) |

## Estructura del Proyecto

```
├── src/
│   ├── main.c              # Demo principal (efectos + melodías)
│   ├── startup.s           # Inicialización del sistema
│   └── simple_vectors.s    # Vectores de interrupción 6502
├── libs/                   # Librerías (clonar por separado)
│   ├── uart-6502-cc65/     # UART optimizado en ASM
│   ├── timer-6502-cc65/    # Timer de precisión
│   └── sid/                # Librería SID 6581
├── vhdl/                   # Código VHDL para FPGA
│   ├── Board.vhd           # Top-level del sistema
│   ├── sid_wrapper.vhd     # Wrapper del SID
│   ├── uart_wrapper.vhd    # Wrapper UART
│   └── NetSID/             # Implementación SID 6581
├── config/
│   └── fpga.cfg            # Configuración del linker cc65
├── scripts/
│   └── bin2rom3.py         # Conversor BIN → VHDL
├── build/                  # Archivos compilados (generado)
├── output/                 # ROM generada (generado)
└── makefile                # Compilación
```

## 🎵 Chip de Sonido SID 6581

El SID está mapeado en `$D400-$D41C`, compatible con el Commodore 64.

### Características
- **3 voces** independientes
- **Formas de onda**: Triangle, Sawtooth, Pulse, Noise
- **Envelope ADSR** por voz
- **Filtro multimodo**: Low-pass, Band-pass, High-pass
- **Modulación**: Sync y Ring

### Ejemplo de Uso

```c
#include <stdint.h>
#include "timer.h"
#include "uart.h"
#include "sid.h"

int main(void) {
    sid_init();
    SID_MODE_VOL = 0x0F;  /* Volumen máximo */
    
    /* Configurar ADSR */
    SID_V1_AD = 0x00;     /* Attack=0, Decay=0 */
    SID_V1_SR = 0xF0;     /* Sustain=15, Release=0 */
    
    /* Tocar nota A4 (440 Hz) con onda de pulso */
    SID_V1_PW_LO = 0x00;
    SID_V1_PW_HI = 0x08;  /* Pulse width 50% */
    SID_V1_FREQ_LO = NOTE_A4 & 0xFF;
    SID_V1_FREQ_HI = NOTE_A4 >> 8;
    SID_V1_CTRL = SID_PULSE | SID_GATE;
    
    delay_ms(500);
    
    SID_V1_CTRL = SID_PULSE;  /* Gate off */
    
    while(1);
    return 0;
}
```

## 🚀 Instalación

### 1. Clonar el repositorio
```bash
git clone https://github.com/nelsama/test_SID_6502_TN-9k.git
cd test_SID_6502_TN-9k
```

### 2. Clonar las librerías necesarias
```bash
cd libs
git clone https://github.com/nelsama/uart-6502-cc65.git
git clone https://github.com/nelsama/timer-6502-cc65.git
git clone https://github.com/nelsama/sid-6502-cc65.git sid
cd ..
```

### 3. Compilar
```bash
make
```

### 4. Cargar en FPGA
Copiar `output/rom.vhd` al proyecto FPGA y sintetizar con Gowin IDE.

## Requisitos

- [cc65](https://cc65.github.io/) - Compilador C para 6502
- Python 3 - Para el script bin2rom3.py
- FPGA Tang Nano 9K
- Gowin IDE para síntesis FPGA

## Uso de Librerías cc65

Este template incluye un startup que inicializa correctamente el runtime de cc65.
Puedes usar librerías estándar sin problemas:

```c
#include <stdlib.h>
#include <string.h>

int main(void) {
    char buffer[32];
    int random_num;
    
    srand(12345);
    random_num = rand() % 100;
    
    strcpy(buffer, "Hola 6502!");
    
    // ...
}
```

## Mapa de Memoria

| Región | Dirección | Tamaño | Descripción |
|--------|-----------|--------|-------------|
| Zero Page | $0002-$00FF | 254 bytes | Variables rápidas cc65 |
| RAM | $0200-$3FFF | 16 KB | RAM principal + DATA + Stack |
| ROM | $8000-$9FFF | 8 KB | Código del programa |
| Vectores | $9FFA-$9FFF | 6 bytes | NMI, RESET, IRQ |
| GPIO/LEDs | $C000-$C003 | 4 bytes | Puertos de E/S |
| UART | $C020-$C021 | 2 bytes | Comunicación serial |
| Timer | $C030-$C03C | 13 bytes | Timer de precisión |
| **SID** | $D400-$D41C | 29 bytes | Chip de sonido |

## 🔊 Salida de Audio

**Pin:** 33 (PWM)

**Circuito recomendado:**
```
Pin 33 ──[10kΩ]──┬──[100nF]── GND
                 │
                 └── Amplificador/Altavoz
```

## 📊 Uso de ROM

| Módulo | Tamaño |
|--------|--------|
| main.c (demo) | ~4.2 KB |
| timer | ~626 bytes |
| sid | ~465 bytes |
| uart | ~105 bytes |
| startup | ~139 bytes |
| cc65 runtime | ~835 bytes |
| **Total** | **~6.4 KB / 8 KB (79%)** |

## Licencia

MIT
