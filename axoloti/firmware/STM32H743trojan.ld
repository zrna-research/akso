ENTRY(xpatch_init)

MEMORY
{

SRAM (rwx)  : ORIGIN = 0x24000000, LENGTH = 384k  /* first 64kB of SRAM1 */
SRAM2 (rwx) : ORIGIN = 0x2001C000, LENGTH =  8k  /* first 8kB in SRAM2 */
SRAM3 (rwx) : ORIGIN = 0x24060000, LENGTH = 118k  /* whole of SRAM3 */
CCMSRAM (rw) : ORIGIN = 0x20000000, LENGTH = 0x1d000 /* CCM - ccmramend */
SDRAM : org = 0xC0000000, len = 0x2000000

flash0 (rx) : org = 0x08000000, len = 2M        /* Flash bank1+bank2 */
flash1 (rx) : org = 0x08000000, len = 1M        /* Flash bank 1 */
flash2 (rx) : org = 0x08100000, len = 1M        /* Flash bank 2 */
flash3 (rx) : org = 0x00000000, len = 0
flash4 (rx) : org = 0x00000000, len = 0
flash5 (rx) : org = 0x00000000, len = 0
flash6 (rx) : org = 0x00000000, len = 0
flash7 (rx) : org = 0x00000000, len = 0
ram0   (wx) : org = 0x24000000, len = 384k      /* AXI SRAM */
ram0a  (wx) : org = 0x2407e800, len = 8k      /* AXI SRAM Upper a */
ram0b  (wx) : org = 0x24060000, len = 118k      /* AXI SRAM Upper b */
ram1   (wx) : org = 0x30000000, len = 256k      /* AHB SRAM1+SRAM2 */
ram2   (wx) : org = 0x30000000, len = 288k      /* AHB SRAM1+SRAM2+SRAM3 */
ram3   (wx) : org = 0x30040000, len = 32k       /* AHB SRAM3 */
ram4   (wx) : org = 0x38000000, len = 64k       /* AHB SRAM4 */
ram5   (wx) : org = 0x20000000, len = 128k      /* DTCM-RAM */
ram6   (wx) : org = 0x00000000, len = 64k       /* ITCM-RAM */
ram7   (wx) : org = 0x38800000, len = 4k        /* BCKP SRAM */
ram8   (wx) : org = 0x2001d000, len = 10k       /* DTCM-RAM end */
}

SECTIONS {

	header : {
		LONG(0x32787861) /* signature "axx2"  */
		LONG(patch_root_chunk)
	} > SRAM

    /* .text : ALIGN(16) SUBALIGN(16) */
    /* { */
    /*     * (.boot); */
    /*     * (.text); */
    /*     etext = .; */
    /* } > SRAM */

    /* constructors : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     PROVIDE(__ctor_array_start = .); */
    /*     KEEP(*(SORT(.init_array.*))) */
    /*     KEEP(*(.init_array)) */
    /*     PROVIDE(__ctor_array_end = .); */
    /* } > SRAM */

    /* destructors : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     PROVIDE(__dtor_array_start = .); */
    /*     KEEP(*(.fini_array)) */
    /*     KEEP(*(SORT(.fini_array.*))) */
    /*     PROVIDE(__dtor_array_end = .); */
    /* } > SRAM */

    /* .rodata : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     * (.rodata); */
    /*     * (.rodata.*); */
    /* } > SRAM */

    /* .data : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     sdata = .; */
    /*     * (.data); */
    /*     edata = .; */
    /* } > SRAM */

    /* .sram2 (NOLOAD): ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     * (.sram2); */
    /*     * (.sram2.*); */
    /* } > SRAM */

    /* .bss (NOLOAD): ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     PROVIDE(_pbss_start = .); */
    /*     sbss = .; */
    /*     * (.bss); */
    /*     ebss = .; */
    /*     PROVIDE(_pbss_end = .); */
    /* } > SRAM */

    /* .sdram.data : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     KEEP(*(.sdramdata)) */
    /*     KEEP(*(.sdramdata.*)) */
    /* } > SDRAM */

    /* .sdram (NOLOAD) : ALIGN(4) SUBALIGN(4) */
    /* { */
    /*     KEEP(*(.sdram)) */
    /*     KEEP(*(.sdram.*)) */
    /*     PROVIDE(_sdram_dyn_start = .); */
    /*     PROVIDE(_sdram_dyn_end = ORIGIN(SDRAM) + LENGTH(SDRAM)); */
    /* } > SDRAM */


 /*
    .ARM.exidx :
    {
       __exidx_start = .;
       *(.ARM.exidx* .gnu.linkonce.armexidx.*)
       __exidx_end = .;
    } >CCMSRAM
    _etext = .;
*/
}


/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

/*Flash region to be used for exception vectors. */
REGION_ALIAS("VECTORS_FLASH", SRAM);
REGION_ALIAS("VECTORS_FLASH_LMA", SRAM);

/* Flash region to be used for constructors and destructors.*/
REGION_ALIAS("XTORS_FLASH", SRAM);
REGION_ALIAS("XTORS_FLASH_LMA", SRAM);

/* Flash region to be used for code text.*/
REGION_ALIAS("TEXT_FLASH", SRAM);
REGION_ALIAS("TEXT_FLASH_LMA", SRAM);

/* Flash region to be used for read only data.*/
REGION_ALIAS("RODATA_FLASH", SRAM);
REGION_ALIAS("RODATA_FLASH_LMA", SRAM);

/* Flash region to be used for various.*/
REGION_ALIAS("VARIOUS_FLASH", SRAM);
REGION_ALIAS("VARIOUS_FLASH_LMA", SRAM);

/* Flash region to be used for RAM(n) initialization data.*/
REGION_ALIAS("RAM_INIT_FLASH_LMA", SRAM);

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts*/
REGION_ALIAS("MAIN_STACK_RAM", SRAM);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", SRAM);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", SRAM);
REGION_ALIAS("DATA_RAM_LMA", SRAM);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", SRAM);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", SRAM);


/* SECTIONS */
/* { */
/*     .entry_section : */
/*     { */
/*         LONG(0x32787861) /\* signature "axx2"  *\/ */
/*         LONG(patch_root_chunk) */
/*         *(.entry_section) */
/*         *(.entry_section.*) */
/*         LONG(0) */
/*         LONG(0x12345678) */
/*         . = 0x0200; */
/*     } > flash0 */
/* } */

/* Generic rules inclusion.*/
INCLUDE rules.ld
