    .globl _switch_stack
    .p2align 2
_switch_stack: ; void (char **old_stack_top (x0), char **new_stack_top (x1))
    ; push registers
    sub sp, sp, #88  ; make room for 11 8-byte registers
    stp x19, x20, [sp]
    stp x21, x22, [sp, #16]
    stp x23, x24, [sp, #32]
    stp x25, x26, [sp, #48]
    stp x27, x28, [sp, #64]
    str lr, [sp, #80]
    ; store stack pointer into the address pointed at by first argument (x0)
    str sp, [x0]
    ; restore stack pointer from the address pointed at by second argument (x1)
    ldr sp, [x1]
    ; pop registers
    ldr lr, [sp, #80]
    stp x27, x28, [sp, #64]
    stp x25, x26, [sp, #48]
    stp x23, x24, [sp, #32]
    stp x21, x22, [sp, #16]
    stp x19, x20, [sp]
    add sp, sp, #88  ; return stack pointer to its original position
    ret
