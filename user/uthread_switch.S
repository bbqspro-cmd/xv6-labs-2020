	.text

/*
* save the old thread's registers,
* restore the new thread's registers.
*/

	.globl thread_switch
thread_switch:
	#a0 = old thread tcontext pointer 
	#a1 = new thread tcontext pointer

	#first step: 把当前所有的callee-saved寄存器保存到旧线程 tcontext  
    /* YOUR CODE HERE */
    sd ra, 0(a0)
    sd sp, 8(a0)
    sd s0, 16(a0)
    sd s1, 24(a0)
    sd s2, 32(a0)
    sd s3, 40(a0)
    sd s4, 48(a0)
    sd s5, 56(a0)
    sd s6, 64(a0)
    sd s7, 72(a0)
    sd s8, 80(a0)
    sd s9, 88(a0)
    sd s10, 96(a0)
    sd s11, 104(a0)

	#second step：从新线程的 tcontext 恢复寄存器

    ld ra, 0(a1)
    ld sp, 8(a1)
    ld s0, 16(a1)
    ld s1, 24(a1)
    ld s2, 32(a1)
    ld s3, 40(a1)
    ld s4, 48(a1)
    ld s5, 56(a1)
    ld s6, 64(a1)
    ld s7, 72(a1)
    ld s8, 80(a1)
    ld s9, 88(a1)
    ld s10, 96(a1)
    ld s11, 104(a1)

	# 第三步：ret →跳转到 ra
    # 如果是第一次切换到该线程，ra = func，所以直接进入 func 执行
    # 如果是恢复旧线程，ra = 上次 thread_switch 保存的返回地址
    ret    /* return to ra */
