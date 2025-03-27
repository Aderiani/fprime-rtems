/* Custom linker script for GR740 with F´ */

/* Define memory base and size */
MEMORY
{
  ram : ORIGIN = 0x40000000, LENGTH = 128M
}

/* Set base address */
. = 0x40000000;

/* Entry point */
ENTRY(Init)

/* Sections */
SECTIONS
{
  .text 0x40000000 :
  {
    *(.text .text.*)
    . = ALIGN(16);
  } > ram AT > ram

  .rtemsroset :
  {
    *(.rtemsroset .rtemsroset.*)
    . = ALIGN(16);
  } > ram AT > ram

  .data :
  {
    *(.data .data.*)
    . = ALIGN(16);
  } > ram AT > ram

  .got :
  {
    *(.got .got.*)
    . = ALIGN(16);
  } > ram AT > ram

  .bss (NOLOAD) :
  {
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(16);
    _end = .;
  } > ram AT > ram

  /* Stack and heap */
  _stack_start = ORIGIN(ram) + LENGTH(ram) - 4K;  /* 0x47FFFFF0 */
  _heap_start = _end;
}

/* Include base script after overrides (optional) */
INCLUDE linkcmds.base