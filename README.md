# 6502 SID Project - Tang Nano 9K

🚀 **Proyecto 6502 con chip de sonido SID 6581** sobre FPGA Tang Nano 9K.

Sistema embebido retro con CPU 6502 y el legendario chip de sonido del Commodore 64.

## Características

- ✅ CPU 6502 @ 3.375 MHz en FPGA Tang Nano 9K
- ✅ **Chip de sonido SID 6581** (3 voces, filtros, ADSR)
- ✅ Control de 6 LEDs 
- ✅ Comunicación UART para debug
- ✅ Timer de precisión con microsegundos
- ✅ Interfaz I2C Master
- ✅ Compilación con cc65
- ✅ **Compatible con librerías estándar de cc65** (stdlib, string, etc.)
- ✅ Startup con copydata y zerobss
- ✅ Librerías en C listas para usar

## Hardware Soportado

| Componente | Dirección | Descripción |
|------------|-----------|-------------|
| PORT_1 | $C000 | Puerto GPIO 1 (8 bits bidireccional) |
| PORT_2 / LEDs | $C001 | Puerto GPIO 2 / LEDs (6 bits) |
| CONF_PORT_1 | $C002 | Configuración Puerto 1 (0=salida, 1=entrada) |
| CONF_PORT_2 | $C003 | Configuración Puerto 2 (0=salida, 1=entrada) |
| I2C | $C010-$C014 | Interfaz I2C Master |
| UART | $C020-$C023 | Comunicación serial 115200 baud |
| Timer | $C030-$C03C | Timer de precisión / RTC |
| **SID 6581** | $D400-$D41F | Chip de sonido (compatible C64) |

## Estructura del Proyecto

```
├── src/
│   ├── main.c              # Programa principal (edita aquí)
│   ├── startup.s           # Inicialización del sistema
│   └── simple_vectors.s    # Vectores de interrupción 6502
├── libs/                   # Librerías
│   ├── uart/               # Comunicación serial
│   └── sid/                # Chip de sonido SID 6581
├── config/
│   └── fpga.cfg            # Configuración del linker cc65
├── scripts/
│   └── bin2rom3.py         # Conversor BIN → VHDL
├── build/                  # Archivos compilados (generado)
├── output/                 # ROM generada (generado)
└── makefile                # Compilación
```

## 🎵 Chip de Sonido SID 6581

El SID está mapeado en `$D400-$D41F`, compatible con el Commodore 64.

### Características
- **3 voces** independientes
- **Formas de onda**: Triangle, Sawtooth, Pulse, Noise
- **Envelope ADSR** por voz
- **Filtro multimodo**: Low-pass, Band-pass, High-pass
- **Modulación**: Sync y Ring

### Ejemplo de Uso

```c
#include "../libs/sid/sid.h"

int main(void) {
    sid_init();
    sid_set_volume(15);
    
    /* Configurar voz 1: onda de pulso */
    sid_set_waveform(0, SID_PULSE);
    sid_set_pulse_width(0, 2048);
    sid_set_adsr(0, 0, 9, 0, 0);
    
    /* Tocar nota A4 (440 Hz) */
    sid_play_note(0, NOTE_A4);
    
    return 0;
}
```

Ver [libs/sid/README.md](libs/sid/README.md) para documentación completa.

## Cómo Usar

1. **Clona o descarga** este repositorio
2. **Edita** `src/main.c` con tu código
3. **Agrega librerías** en la carpeta `libs/` según necesites
4. **Compila** con `make`
5. **Carga** `output/rom.vhd` en tu proyecto FPGA

## Compilación

### Requisitos previos
- [cc65](https://cc65.github.io/) instalado en `D:\cc65`
- Python 3 para el script de conversión

### Compilar
```bash
make
```

### Limpiar
```bash
make clean
```

### Cargar en FPGA
Copiar `output/rom.vhd` al proyecto FPGA y sintetizar.

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
| GPIO | $C000-$C003 | 4 bytes | Puertos de E/S |
| I2C | $C010-$C014 | 5 bytes | Interfaz I2C Master |
| UART | $C020-$C023 | 4 bytes | Comunicación serial |
| Timer | $C030-$C03C | 13 bytes | Timer de precisión |
| **SID** | $D400-$D41F | 32 bytes | Chip de sonido |

## Salida de Audio

**Pin:** 33 (PWM)

**Circuito recomendado:**
```
Pin 33 ──[10kΩ]──┬──[100nF]── GND
                 │
                 └── Amplificador/Altavoz
```

## Archivos del Sistema

| Archivo | Descripción |
|---------|-------------|
| `startup.s` | Inicializa stack, copydata, zerobss y llama a main |
| `simple_vectors.s` | Define vectores NMI, RESET, IRQ |
| `fpga.cfg` | Mapa de memoria para el linker |

## Requisitos

- [cc65](https://cc65.github.io/) - Compilador C para 6502
- Python 3 - Para el script bin2rom3.py
- FPGA Tang Nano 9K

## Licencia

MIT
