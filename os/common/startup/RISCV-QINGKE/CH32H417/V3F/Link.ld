ENTRY( _start )

__stack_size = 2048;

PROVIDE( _stack_size = __stack_size );


MEMORY
{


	/*FLASH, RAM, and RAM-CODE can be freely configured according to project 
	requirements;Please refer to the datasheet manual for specific allocation*/
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K             /* zero_wait flah ,and loaded into RAM_CODE for running */
	RAM_CODE (xrw) : ORIGIN = 0x20180000 - 256K, LENGTH = 256K         /* RAM for running code*/
	RAM (xrw) : ORIGIN = 0x20180000 - 256K - 128K + 1024, LENGTH = 128K - 1024 /* RAM for .data .bss section and other*/
	RAM_LOAD (xrw) : ORIGIN = 0x20180000 - 256K - 128K, LENGTH = 1024         /* RAM for .load section*/
}

SECTIONS
{
	.start :
	{
		_sinit = .;
		. = ALIGN(4);
		KEEP(*(SORT_NONE(.start)))
		. = ALIGN(4);
		_einit = .;
	} >FLASH AT>FLASH

    .loadcodelalign : 
    {       
        . = ALIGN(4);
        PROVIDE(_loadcode_lma = .); 
    } >FLASH AT>FLASH 

	.loadcode :
	{
		. = ALIGN(4);
        PROVIDE(_loadcode_vma_start = .);
		*(.load);
		KEEP(*(SORT_NONE(.handle_reset)))
		. = ALIGN(4); 
		PROVIDE(_loadcode_vma_end = .);
	}>RAM_LOAD AT>FLASH

    .highcodelalign : 
    {       
        . = ALIGN(4);
        PROVIDE(_highcode_lma = .); 
    } >FLASH AT>FLASH 
    
    .highcode : 
    {
        . = ALIGN(1024);
        PROVIDE(_highcode_vma_start = .);
		*(.irq_entry);
		. = ALIGN(4);
		PROVIDE(vector_start = .);
        KEEP(*(.vector))
		PROVIDE(__text_base__ = .);
		*(.text)
		*(.text.*)
		PROVIDE(__text_end__ = .);
		PROVIDE(__rodata_base__ = .);
		*(.rodata)
		*(.rodata*)
		PROVIDE(__rodata_end__ = .);
        *(.sdata2.*)
		*(.glue_7)
		*(.glue_7t)
		*(.gnu.linkonce.t.*)
		. = ALIGN(4); 
        PROVIDE(_highcode_vma_end = .);
    } >RAM_CODE AT>FLASH

	.text :
	{
	} >FLASH AT>FLASH 

    .highcodelalign1 : 
    {       
        . = ALIGN(4);
        PROVIDE(_highcode_lma1 = .); 
    } >FLASH AT>FLASH 

	.fini :
	{
	    . = ALIGN(4);
        PROVIDE(_highcode_vma_start1 = .);
		KEEP(*(SORT_NONE(.fini)))
		. = ALIGN(4);
	} >RAM_CODE AT>FLASH

	PROVIDE( _etext = . );
	PROVIDE( _eitcm = . );	

	.preinit_array  :
	{
	  PROVIDE_HIDDEN (__preinit_array_start = .);
	  KEEP (*(.preinit_array))
	  PROVIDE_HIDDEN (__preinit_array_end = .);
	} >RAM_CODE AT>FLASH 
	
	.init_array     :
	{
	  PROVIDE_HIDDEN (__init_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
	  KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
	  PROVIDE_HIDDEN (__init_array_end = .);
	} >RAM_CODE AT>FLASH 
	
	.fini_array     :
	{
	  PROVIDE_HIDDEN (__fini_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
	  KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
	  PROVIDE_HIDDEN (__fini_array_end = .);
	} >RAM_CODE AT>FLASH 
	
	.ctors          :
	{
	  /* gcc uses crtbegin.o to find the start of
	     the constructors, so we make sure it is
	     first.  Because this is a wildcard, it
	     doesn't matter if the user does not
	     actually link against crtbegin.o; the
	     linker won't look for a file to match a
	     wildcard.  The wildcard also means that it
	     doesn't matter which directory crtbegin.o
	     is in.  */
	  KEEP (*crtbegin.o(.ctors))
	  KEEP (*crtbegin?.o(.ctors))
	  /* We don't want to include the .ctor section from
	     the crtend.o file until after the sorted ctors.
	     The .ctor section from the crtend file contains the
	     end of ctors marker and it must be last */
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
	  KEEP (*(SORT(.ctors.*)))
	  KEEP (*(.ctors))
	} >RAM_CODE AT>FLASH 
	
	.dtors          :
	{
	  KEEP (*crtbegin.o(.dtors))
	  KEEP (*crtbegin?.o(.dtors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  KEEP (*(SORT(.dtors.*)))
	  KEEP (*(.dtors))
	  . = ALIGN(4); 
      PROVIDE(_highcode_vma_end1 = .);
	} >RAM_CODE AT>FLASH 

	.dalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
	} >RAM AT>FLASH	

	.dlalign :
	{
		. = ALIGN(4); 
		PROVIDE(_data_lma = .);
	} >FLASH AT>FLASH

	.data :
	{
		. = ALIGN(4);
    	*(.gnu.linkonce.r.*)
		PROVIDE(__data_base__ = .);
    	*(.data .data.*)
		PROVIDE(__data_end__ = .);
    	*(.gnu.linkonce.d.*)
		. = ALIGN(8);
    	PROVIDE( __global_pointer$ = . + 0x800 );
    	*(.sdata .sdata.*)
		*(.sdata2.*)
    	*(.gnu.linkonce.s.*)
    	. = ALIGN(8);
    	*(.srodata.cst16)
    	*(.srodata.cst8)
    	*(.srodata.cst4)
    	*(.srodata.cst2)
    	*(.srodata .srodata.*)
    	. = ALIGN(4);
		PROVIDE( _edata = .);
	} >RAM AT>FLASH

	.bss :
	{
		. = ALIGN(4);
		PROVIDE( _sbss = .);
  	    *(.sbss*)
        *(.gnu.linkonce.sb.*)
		PROVIDE(__bss_base__ = .);
		*(.bss*)
		PROVIDE(__bss_end__ = .);
     	*(.gnu.linkonce.b.*)		
		*(COMMON*)
		. = ALIGN(4);
		PROVIDE( _ebss = .);
	} >RAM AT>FLASH

	PROVIDE( _end = _ebss);
	PROVIDE( end = . );
	PROVIDE( __heap_base__ = . );

    .stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size :
    {
        PROVIDE( _heap_end = . );
		PROVIDE( __heap_end__ = . );

        . = ALIGN(4);
        PROVIDE(_susrstack = . );
		PROVIDE( __process_stack_base__ = . );
		PROVIDE( __main_thread_stack_base__ = . );
        . = . + __stack_size;
        PROVIDE( _eusrstack = .);
		PROVIDE( __process_stack_end__ = . );
		PROVIDE( __main_thread_stack_end__ = . );
    } >RAM 

}



