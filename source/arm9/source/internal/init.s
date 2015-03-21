    IMPORT __use_no_semihosting
    IMPORT __use_two_region_memory
;    IMPORT __use_realtime_heap
    
    EXPORT _sys_exit
    EXPORT _ttywrch
    EXPORT __user_initial_stackheap

    AREA     globals,CODE,READONLY
    
_sys_exit
	b {pc}
	
_ttywrch
	bx lr
	
__user_initial_stackheap
	;r0 �еĶѻ�ַ
	;r1 �еĶ�ջ��ַ������ջ���е���ߵ�ַ
	;r2 �еĶ�����
	;r3 �еĶ�ջ���ƣ�����ջ���е���͵�ַ
	
	IMPORT bottom_of_heap ; defined in heap.s

	ldr r0,=bottom_of_heap
	ldr r2,=0x023ff000-0x23e00-0x13e00-0x21800-0x10000-0xf00 ;
	ldr r1,=0x0b003f00
	ldr r3,=0x0b000000
	
	bx lr

    END
    
