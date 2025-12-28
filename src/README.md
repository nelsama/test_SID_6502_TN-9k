# C�digo Fuente

## Archivos

| Archivo | Descripci�n |
|---------|-------------|
| `main.c` | **Programa principal** - Edita aqu� tu c�digo |
| `startup.s` | Inicializaci�n del sistema (copydata, zerobss) |
| `simple_vectors.s` | Vectores de interrupci�n NMI, RESET, IRQ |

## main.c

Este es el archivo principal donde escribes tu programa.

### Registros de Hardware

| Registro | Direcci�n | Descripci�n |
|----------|-----------|-------------|
| `PORT_SALIDA_LED` | $C001 | Salida para 6 LEDs (bits 0-5) |
| `CONF_PORT_SALIDA_LED` | $C003 | Config: 0=salida, 1=entrada |

## startup.s

C�digo de inicializaci�n que se ejecuta antes de `main()`:

1. Configura el stack pointer del 6502
2. Configura el software stack de cc65
3. Ejecuta `copydata` - copia variables inicializadas de ROM a RAM
4. Ejecuta `zerobss` - inicializa variables BSS a cero
5. Llama a `main()`

**No necesitas modificar este archivo.**

## simple_vectors.s

Define los vectores de interrupci�n del 6502:

| Vector | Direcci�n | Funci�n |
|--------|-----------|---------|
| NMI | $9FFA | Non-Maskable Interrupt |
| RESET | $9FFC | Apunta a startup |
| IRQ | $9FFE | Interrupt Request |
