; ============================================
; simple_vectors.s - Vectores de interrupción 6502
; ============================================
; Define los vectores NMI, RESET e IRQ
; El vector RESET apunta al startup (_init)
; ============================================

.import _init        ; Punto de entrada del startup

.segment "CODE"

; Manejadores de interrupción (vacíos por defecto)
nmi_handler:
    rti

irq_handler:
    rti

.segment "VECTORS"

; Tabla de vectores del 6502 ($9FFA-$9FFF)
.addr   nmi_handler  ; $9FFA: NMI
.addr   _init        ; $9FFC: RESET -> startup
.addr   irq_handler  ; $9FFE: IRQ
