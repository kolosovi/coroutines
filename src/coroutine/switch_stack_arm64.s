    .globl _switch_stack
    .p2align 2
_switch_stack: ; void (char **old_stack_top (x0), char **new_stack_top (x1))
    ; make room for 11 8-byte registers (+ 8 bytes to keep stack pointer 16-byte aligned)
    sub sp, sp, #0x60
    ; push registers
    stp x19, x20, [sp, #0x00]
    stp x21, x22, [sp, #0x10]
    stp x23, x24, [sp, #0x20]
    stp x25, x26, [sp, #0x30]
    stp x27, x28, [sp, #0x40]
    str lr, [sp, #0x50]
    ; store stack pointer into the address pointed at by first argument (x0).
    ; third argument (x2) is used as a temporary register to store sp.
    mov x2, sp
    str x2, [x0]
    ; restore stack pointer from the address pointed at by second argument (x1).
    ; third argument (x2) is used as a temporary register to store sp value read from stack.
    ldr x2, [x1]
    mov sp, x2
    ; pop registers
    ldp x19, x20, [sp, #0x00]
    ldp x21, x22, [sp, #0x10]
    ldp x23, x24, [sp, #0x20]
    ldp x25, x26, [sp, #0x30]
    ldp x27, x28, [sp, #0x40]
    ldr lr, [sp, #0x50]
    ; return stack pointer to its original position
    add sp, sp, #0x60
    ret
