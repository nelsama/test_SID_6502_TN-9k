# fpga-6502

Computador retro basado en el procesador **MOS 6502** implementado en una **Sipeed Tang Nano 9K** (Gowin GW1NR-9). Incluye CPU, memoria RAM/ROM, puertos GPIO bidireccionales, comunicación I2C, UART y **chip de sonido SID 6581**.

## 🏗️ Arquitectura del Sistema

```
┌──────────────────────────────────────────────────────────────────┐
│                         Board.vhd (Top-Level)                    │
│                                                                  │
│  CLOCK_27MHz ──▶ [CLKDIV] ──▶ 6.75 MHz ──▶ /2 ──▶ 3.375 MHz     │
│                      │                                           │
│                      └──▶ /7 ──▶ ~1 MHz (SID)                    │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                      CPU 6502 (cpu65xx_fast)                │ │
│  │              Cycle-exact, table-driven implementation       │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                              │                                   │
│                       [Data Bus Mux]                             │
│         ┌────────────────────┼────────────────────┐              │
│         ▼            ▼       ▼       ▼            ▼              │
│   ┌──────────┐ ┌──────────┐ ┌──────────────┐ ┌──────────┐       │
│   │   RAM    │ │   ROM    │ │  Puertos I/O │ │ SID 6581 │       │
│   │  16 KB   │ │   8 KB   │ │GPIO+I2C+UART │ │  Audio   │       │
│   └──────────┘ └──────────┘ └──────────────┘ └────┬─────┘       │
│                                                   │ PWM          │
│                                              [Audio Out]         │
└──────────────────────────────────────────────────────────────────┘
```

## 🗺️ Mapa de Memoria

| Rango | Tamaño | Descripción |
|-------|--------|-------------|
| `$0000 - $3FFF` | 16 KB | **RAM** (Zero Page, Stack, memoria de trabajo) |
| `$4000 - $7FFF` | 16 KB | *No usado* |
| `$8000 - $9FFF` | 8 KB | **ROM** (Código del programa) |
| `$A000 - $BFFF` | 8 KB | *No usado* |
| `$C000` | 1 byte | **Puerto 1** - Datos (bidireccional) |
| `$C001` | 1 byte | **Puerto 2** - Datos (bidireccional) |
| `$C002` | 1 byte | **Config Puerto 1** (0=salida, 1=entrada) |
| `$C003` | 1 byte | **Config Puerto 2** (0=salida, 1=entrada) |
| `$C010 - $C017` | 8 bytes | **Registros I2C Master** |
| `$C020 - $C023` | 4 bytes | **Registros UART** |
| `$C030 - $C03F` | 16 bytes | **Timer de precisión** |
| `$C040 - $C05F` | 32 bytes | **SID 6581** (chip de sonido) |
| `$FFFA - $FFFF` | 6 bytes | **Vectores** (mapeados a ROM $9FFA-$9FFF) |

### Registros I2C

| Dirección | Registro |
|-----------|----------|
| `$C010` | Prescaler LSB |
| `$C011` | Prescaler MSB |
| `$C012` | Control |
| `$C013` | TX/RX Data |
| `$C014` | Command/Status |

### Registros UART (115200 baud, 8N1)

| Dirección | Lectura | Escritura |
|-----------|---------|----------|
| `$C020` | RX Data | TX Data |
| `$C021` | Status | Control |

### Registros Timer/RTC

| Dirección | Registro | Descripción |
|-----------|----------|-------------|
| `$C030` | TICK_0 | Contador ticks byte 0 (LSB) |
| `$C031` | TICK_1 | Contador ticks byte 1 |
| `$C032` | TICK_2 | Contador ticks byte 2 |
| `$C033` | TICK_3 | Contador ticks byte 3 (MSB) |
| `$C034` | TIMER_LO | Timer countdown low (R/W) |
| `$C035` | TIMER_HI | Timer countdown high (R/W) |
| `$C036` | TIMER_CTL | Control/Status |
| `$C037` | PRESCALER | Prescaler del timer |
| `$C038` | USEC_0 | Microsegundos byte 0 (LSB) |
| `$C039` | USEC_1 | Microsegundos byte 1 |
| `$C03A` | USEC_2 | Microsegundos byte 2 |
| `$C03B` | USEC_3 | Microsegundos byte 3 (MSB) |
| `$C03C` | LATCH_CTL | Control de latch |

**TIMER_CTL bits:** EN(0), IRQ_EN(1), REPEAT(2), IRQ_FLAG(3), ZERO(7)

**Status bits (lectura):** TX_READY(0), RX_VALID(1), TX_BUSY(2), RX_ERROR(3), RX_OVERRUN(4)

### Registros SID 6581

El chip de sonido SID está mapeado en `$C040-$C05F`.

| Dirección | Registro | Descripción |
|-----------|----------|-------------|
| `$C040` | FREQ_LO_1 | Voz 1 - Frecuencia low byte |
| `$C041` | FREQ_HI_1 | Voz 1 - Frecuencia high byte |
| `$C042` | PW_LO_1 | Voz 1 - Pulse width low byte |
| `$C043` | PW_HI_1 | Voz 1 - Pulse width high (bits 0-3) |
| `$C044` | CTRL_1 | Voz 1 - Control (Gate, Sync, Ring, Test, Waveform) |
| `$C045` | AD_1 | Voz 1 - Attack/Decay |
| `$C046` | SR_1 | Voz 1 - Sustain/Release |
| `$C047-$C04D` | | Voz 2 (misma estructura) |
| `$C04E-$C054` | | Voz 3 (misma estructura) |
| `$C055` | FC_LO | Filtro - Cutoff frequency low (bits 0-2) |
| `$C056` | FC_HI | Filtro - Cutoff frequency high |
| `$C057` | RES_FILT | Resonancia (4-7) / Selección filtro (0-3) |
| `$C058` | MODE_VOL | Modo filtro (4-7) / Volumen master (0-3) |
| `$C059` | POT_X | Paddle X (solo lectura) |
| `$C05A` | POT_Y | Paddle Y (solo lectura) |
| `$C05B` | OSC3 | Oscilador 3 random (solo lectura) |
| `$C05C` | ENV3 | Envelope 3 (solo lectura) |

**Control Register ($C044, $C04B, $C052) bits:**
- Bit 0: GATE (iniciar/detener envelope)
- Bit 1: SYNC (sincronizar con voz anterior)
- Bit 2: RING (modulación en anillo)
- Bit 3: TEST (desactivar oscilador)
- Bit 4: TRIANGLE
- Bit 5: SAWTOOTH  
- Bit 6: PULSE
- Bit 7: NOISE

## ⚡ Frecuencias de Reloj

| Señal | Frecuencia | Uso |
|-------|------------|-----|
| `CLOCK_27_i` | 27 MHz | Entrada del cristal |
| `system_clk` | 6.75 MHz | Reloj del sistema |
| `cpu_clk` | **3.375 MHz** | Reloj del CPU 6502 |
| `clk_1mhz` | ~0.96 MHz | Reloj del SID 6581 |

## 🔧 Componentes

### CPU 6502
- **Core**: `cpu65xx_fast.vhd` de Peter Wendrich (proyecto FPGA64)
- **Tipo**: Implementación cycle-exact dirigida por tablas
- **Características**: Soporte completo de IRQ, NMI, pipeline opcional

### Memoria
- **RAM**: 16 KB usando Block RAM de Gowin
- **ROM**: 8 KB con programa hardcoded (generada con Python)

### Puertos GPIO
- **Puerto 1**: 8 bits (LVCMOS33, pines 70-77)
- **Puerto 2**: 6 bits (LVCMOS18, pines 10-16)
- Cada pin configurable individualmente como entrada o salida

### Interfaz I2C
- IP Core `I2C_MASTER_Top` de Gowin
- Señal de interrupción disponible

### Sistema de Reset
- Generador de power-on reset (50ms)
- Debouncer para botón de reset externo

### Chip de Sonido SID 6581
- **Implementación**: NetSID (VHDL)
- **3 voces** con formas de onda: Triangle, Sawtooth, Pulse, Noise
- **Filtro multimodo**: Low-pass, Band-pass, High-pass, Notch
- **ADSR envelope** por voz
- **Salida**: PWM (pin 33) → filtro RC → amplificador
- **Frecuencia**: ~1 MHz (derivada de 6.75 MHz)
- **Direccionamiento**: $C040-$C05F (32 bytes)

**Circuito de audio recomendado:**
```
Pin 33 ──[10kΩ]──┬──[100nF]── GND
                 │
                 └── Amplificador/Altavoz
```

## 📌 FPGA Target

| Parámetro | Valor |
|-----------|-------|
| **Placa** | Sipeed Tang Nano 9K |
| **Familia** | Gowin GW1NR |
| **Part Number** | GW1NR-LV9QN88PC6/I5 |
| **Dispositivo** | GW1NR-9C |
| **Paquete** | QN88 |

### Pinout

| Señal | Pin | Tipo | Descripción |
|-------|-----|------|-------------|
| `CLOCK_27_i` | 52 | LVCMOS33 | Reloj de entrada 27 MHz |
| `reset_in` | 4 | LVCMOS18 | Botón de reset (pull-up) |
| `cpu_clk_out` | 35 | LVCMOS33 | Salida reloj CPU 3.375 MHz |
| `uart_tx` | 32 | LVCMOS33 | UART TX → USB-TTL RX |
| `uart_rx` | 31 | LVCMOS33 | UART RX ← USB-TTL TX |
| `sid_audio_out` | 33 | LVCMOS33 | SID Audio PWM output |
| `i2c_scl` | 48 | LVCMOS33 | I2C Clock (pull-up) |
| `i2c_sda` | 49 | LVCMOS33 | I2C Data (pull-up) |
| `port_1[0:7]` | 70-77 | LVCMOS33 | GPIO Puerto 1 (8 bits) |
| `port_2[0:5]` | 10-16 | LVCMOS18 | GPIO Puerto 2 (6 bits) |

## 📂 Estructura del Proyecto

```
src/
├── Board.vhd                 # Top-level entity
├── A6502.vhd                 # Wrapper del CPU
├── Data_bus_mux.vhd          # Multiplexor del bus de datos
├── rom.vhd                   # ROM con programa (8KB)
├── register_8bit.vhd         # Registro para puertos GPIO
├── Reset.vhd                 # Generador de reset
├── Debouncer.vhd             # Anti-rebote
├── i2c_master_interface.vhd  # Wrapper I2C
├── uart.vhd                  # Módulo UART (TX/RX)
├── uart_wrapper.vhd          # Wrapper UART para bus 6502
├── timer_rtc.vhd             # Timer de precisión
├── timer_wrapper.vhd         # Wrapper Timer para bus 6502
├── sid_wrapper.vhd           # Wrapper SID para bus 6502
├── 6502/
│   └── cpu65xx_fast.vhd      # Core del CPU 6502
├── NetSID/                   # Chip de sonido SID 6581
│   └── src/
│       ├── sid_6581.vhd      # Core principal SID
│       ├── sid_voice.vhd     # Generador de voz
│       ├── sid_filters.vhd   # Filtros analógicos
│       └── sid_components.vhd # Componentes auxiliares
├── gowin_clkdiv_*/           # IP divisor de reloj
├── gowin_sp/                 # IP RAM
├── i2c_master/               # IP I2C Master
└── asm/                      # Librerías en ensamblador 6502
    └── timer_lib.asm         # Funciones de timer/delay
impl/                         # Archivos de implementación (generados)
*.gprj                        # Archivo de proyecto Gowin
```

## 🛠️ Compilación y Uso

1. Abrir el proyecto `6502_board_v2_1.gprj` en **Gowin EDA**
2. Ejecutar síntesis
3. Ejecutar Place & Route
4. Generar bitstream
5. Programar la FPGA con el archivo `.fs`

## 🧰 Herramientas Requeridas

- **Gowin EDA** (GOWIN FPGA Designer)
- **Gowin Programmer** para cargar el bitstream

## 📜 Créditos

- **CPU 6502 Core**: Peter Wendrich ([FPGA64 Project](http://www.syntiac.com/fpga64.html))
- **SID 6581 Core**: NetSID Project (implementación VHDL del chip de sonido)

## 📄 Licencia

[Especificar licencia aquí]

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Por favor, abre un issue antes de enviar pull requests.