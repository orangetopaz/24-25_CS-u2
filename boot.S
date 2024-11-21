.section ".text.boot"

.global _start

_start:
  mrc p15, #0, r1, c0, c0, #5     // read value from coprocessors to r1: https://developer.arm.com/documentation/den0042/a/ARM-Processor-modes-and-Registers/Registers/Coprocessor-15
  and r1, r1, #3                  // isolates the first 2 bits of the cp15 Cache Level ID Register (CLIDR) which displays: 00 - no cache, 01 - instrucution-only cache, 01 - data-only cach, 11 - unified cache 
  cmp r1, #0
  bne halt

  mov sp, #0x8000                 // stack pointer, only place I can put variables without trying to murder the pi and kernal (cuz the pi starts booting from there)

  ldr r4, =__bss_start
  ldr r9, =__bss_end
  mov r5, #0
  mov r6, #0
  mov r7, #0
  mov r8, #0
  b       2f

1:
  stmia r4!, {r5-r8}  // stmia = store multiple increment after. all regesters from r5 to r8 get stored to r4, and then the address is incremented by one to the next pos (-ia suffix), and that address is stored back into r4 (the ! after r4)
  // what this does is store the 16 bytes of 0 stored in r5-r8 (as defined in line 15-18) into r4, 0s out the whole bss section (the section for uninitialized c variables)
  // this makes sure that all uninitialized c variables are set to 0 at runtime, else an error gets thrown
  // technicaly you could just use 1 regester to 0 out the whole bss section by looping, but that's less efficient then 4 at a time, 0ing out 4 bytes per loop instead of 16

2:
  cmp r4, r9  // checks every time it loops to function 1, only not branching back to 1 when the r4 address is at the r9 address (when all of the bss regesters have been set to 0 - when bss start = bss end - when the pointer for bss start reaches the address of bss end?)
  blo 1b  // branch if (unsigned) less than (r4 than r9)

  ldr r3, =kernel_main  // gets address of kernel main function from c. the = means address, kinda like & in c funciton param declaration
  blx r3  // branches to address in r3 (kernal_main script)

  b halt  // not neccessary as it would go there anyways, but helps in this case to make absolutly sure that no undefined actions happen
// this is what the unused cores branch to, and what core 0 branches to once kernel_main returns
halt:  
  wfe  // do nothing at low power mode
  b halt  // loop