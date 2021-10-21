.section .init, "ax"
.global enter_cartridge, switch_context
.extern saved_sp
enter_cartridge:
    addi    sp,sp,-12
    la      a5,saved_sp
    sw      sp,0(a5)
    sw      s0,8(sp)
    sw      s1,4(sp)
    sw      ra,0(sp)
    li      a5,0x4000001C
    lw	    a5,0(a5)
    andi	a5,a5,-4
    jalr	a5
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la      a5,saved_sp
    lw      sp,0(a5)
    lw      s0,8(sp)
    lw      s1,4(sp)
    lw      ra,0(sp)
    addi    sp,sp,12
    ret

switch_context:
    addi	sp,sp,-28
    csrr    a5,mepc
    sw	    a5,24(sp)
    sw	    ra,20(sp)
    sw	    t0,16(sp)
    sw	    t1,12(sp)
    sw	    t2,8(sp)
    sw	    s0,4(sp)
    sw	    s1,0(sp)
    sw      sp,0(a0) # Store old sp to first param
    mv      sp,a1 # Move second param to sp
    lw	    a5,24(sp)
    lw	    ra,20(sp)
    lw	    t0,16(sp)
    lw	    t1,12(sp)
    lw	    t2,8(sp)
    lw	    s0,4(sp)
    lw	    s1,0(sp)
    csrw    mepc,a5
    addi    sp,sp,28
    ret
