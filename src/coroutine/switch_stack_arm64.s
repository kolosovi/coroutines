    .globl _switch_stack
    .p2align 2
_switch_stack: ; void (char **old_stack_top (x0), char **new_stack_top (x1))
    ; make room for 20 8-byte registers
    sub sp, sp, #0x100
    ; push registers
    stp  d8,  d9, [sp, #0x00]
    stp d10, d11, [sp, #0x10]
    stp d12, d13, [sp, #0x20]
    stp d14, d15, [sp, #0x30]
    stp x19, x20, [sp, #0x40]
    stp x21, x22, [sp, #0x50]
    stp x23, x24, [sp, #0x60]
    stp x25, x26, [sp, #0x70]
    stp x27, x28, [sp, #0x80]
    stp x29,  lr, [sp, #0x90]
    ; store stack pointer into the address pointed at by first argument (x0).
    ; third argument (x2) is used as a temporary register to store sp.
    mov x2, sp
    str x2, [x0]
    ; restore stack pointer from the address pointed at by second argument (x1).
    ; third argument (x2) is used as a temporary register to store sp value read from stack.
    ldr x2, [x1]
    mov sp, x2
    ; pop registers
    ldp  d8,  d9, [sp, #0x00]
    ldp d10, d11, [sp, #0x10]
    ldp d12, d13, [sp, #0x20]
    ldp d14, d15, [sp, #0x30]
    ldp x19, x20, [sp, #0x40]
    ldp x21, x22, [sp, #0x50]
    ldp x23, x24, [sp, #0x60]
    ldp x25, x26, [sp, #0x70]
    ldp x27, x28, [sp, #0x80]
    ldp x29,  lr, [sp, #0x90]
    ; return stack pointer to its original position
    add sp, sp, #0x100
    ret
