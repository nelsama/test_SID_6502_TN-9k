# ============================================
# Makefile - 6502 Template para Tang Nano 9K
# ============================================
# Compila código C con cc65 y genera ROM VHDL
# Compatible con librerías estándar de cc65
# ============================================

# ============================================
# DIRECTORIOS
# ============================================
SRC_DIR = src
LIB_DIR = libs
BUILD_DIR = build
OUTPUT_DIR = output
CONFIG_DIR = config
SCRIPTS_DIR = scripts

# ============================================
# HERRAMIENTAS CC65
# ============================================
CC65 = cc65
CA65 = ca65
LD65 = ld65
CL65 = cl65
PYTHON = py

# ============================================
# CONFIGURACIÓN
# ============================================
CONFIG = $(CONFIG_DIR)/fpga.cfg
PLATAFORMA = D:\cc65\lib\none.lib
CFLAGS = -t none -O --cpu 6502

# ============================================
# LIBRERÍAS (agregar más aquí)
# ============================================
UART_DIR = $(LIB_DIR)/uart-6502-cc65
SID_DIR = $(LIB_DIR)/sid/src
TIMER_DIR = $(LIB_DIR)/timer-6502-cc65/src

INCLUDES = -I$(UART_DIR) -I$(SID_DIR) -I$(TIMER_DIR)

# ============================================
# ARCHIVOS OBJETO
# ============================================
STARTUP_OBJ = $(BUILD_DIR)/startup.o
MAIN_OBJ = $(BUILD_DIR)/main.o
UART_OBJ = $(BUILD_DIR)/uart.o
SID_OBJ = $(BUILD_DIR)/sid.o
TIMER_OBJ = $(BUILD_DIR)/timer.o
VECTORS_OBJ = $(BUILD_DIR)/simple_vectors.o

OBJS = $(STARTUP_OBJ) $(MAIN_OBJ) $(UART_OBJ) $(SID_OBJ) $(TIMER_OBJ) $(VECTORS_OBJ)

# ============================================
# TARGET PRINCIPAL
# ============================================
TARGET = $(BUILD_DIR)/main.bin

# Regla por defecto
all: dirs $(TARGET) rom
	@echo ========================================
	@echo COMPILADO EXITOSAMENTE
	@echo ========================================
	@echo VHDL: $(OUTPUT_DIR)/rom.vhd
	@echo ========================================

# Crear directorios
dirs:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"

# ============================================
# COMPILACIÓN DE OBJETOS
# ============================================

# Startup (copydata + zerobss)
$(STARTUP_OBJ): $(SRC_DIR)/startup.s
	$(CA65) -t none -o $@ $<

# Main
$(MAIN_OBJ): $(SRC_DIR)/main.c
	$(CL65) -t none $(INCLUDES) -c -o $@ $<

# UART (versión ASM optimizada)
$(UART_OBJ): $(UART_DIR)/uart.s
	$(CA65) -t none -o $@ $<

# SID (versión ensamblador optimizada)
$(SID_OBJ): $(SID_DIR)/sid.s
	$(CA65) -t none -o $@ $<

# Timer (versión ASM optimizada v2.0)
$(TIMER_OBJ): $(TIMER_DIR)/timer.s
	$(CA65) -t none -o $@ $<

# Vectores
$(VECTORS_OBJ): $(SRC_DIR)/simple_vectors.s
	$(CA65) -t none -o $@ $<

# ============================================
# ENLAZADO
# ============================================
$(TARGET): $(OBJS)
	$(LD65) -C $(CONFIG) --start-addr 0x8000 -o $@ $(OBJS) $(PLATAFORMA)

# ============================================
# GENERACIÓN DE ROM
# ============================================
rom: $(TARGET)
	$(PYTHON) $(SCRIPTS_DIR)/bin2rom3.py $(TARGET) -s 8192 --name rom --data-width 8 -o $(OUTPUT_DIR)

# ============================================
# LIMPIEZA
# ============================================
clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(OUTPUT_DIR)\*.vhd" del /q "$(OUTPUT_DIR)\*.vhd"
	@if exist "$(OUTPUT_DIR)\*.bin" del /q "$(OUTPUT_DIR)\*.bin"
	@if exist "$(OUTPUT_DIR)\*.hex" del /q "$(OUTPUT_DIR)\*.hex"

# ============================================
# AYUDA
# ============================================
help:
	@echo ========================================
	@echo Comandos
	@echo ========================================
	@echo   make        - Compilar y generar ROM
	@echo   make clean  - Limpiar archivos
	@echo   make help   - Mostrar esta ayuda
	@echo ========================================

.PHONY: all dirs rom clean help