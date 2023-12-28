## W65C02S Emulator in C++ ##
Based on https://github.com/gianlucag/mos6502 to be used on my 6502 Emulator on the Casio FX-CG50 :) \
soon...

# Copied from original repo:

## Emulator features ##

My project is a simple jump-table based emulator: the actual value of the opcode (let's say 0x80) is used to address a function pointer table, each entry of such table is a C++ function which emulates the behavior of the corresponding real instruction. 

All the 13 addressing modes are emulated:

```
// addressing modes
uint16_t Addr_ACCUM(); // ACCUMULATOR
uint16_t Addr_IMMED(); // IMMEDIATE
uint16_t Addr_ABSOL(); // ABSOLUTE
uint16_t Addr_ZEROP(); // ZERO PAGE
uint16_t Addr_ZRPIX(); // INDEXED-X ZERO PAGE
uint16_t Addr_ZRPIY(); // INDEXED-Y ZERO PAGE
uint16_t Addr_ABSIX(); // INDEXED-X ABSOLUTE
uint16_t Addr_ABSIY(); // INDEXED-Y ABSOLUTE
uint16_t Addr_IMPLI(); // IMPLIED
uint16_t Addr_RELAT(); // RELATIVE
uint16_t Addr_ZPIXN(); // INDEXED-X INDIRECT
uint16_t Addr_ZPINY(); // INDEXED-Y INDIRECT
uint16_t Addr_ABSIN(); // ABSOLUTE INDIRECT
```

All the 151 opcodes are emulated. Since the 6502 CPU uses 8 bit to encode the opcode value it also has a lot of "illegal opcodes" (i.e. opcode values other than the designed 151). Such opcodes perform weird operations, write multiple registers at the same time, sometimes are the combination of two or more "valid" opcodes. Such illegals were used to enforce software copy protection or to discover the exact CPU type. 

The illegals are not supported yet, so instead a simple NOP is executed.


## Inner main loop ##

It's a classic fetch-decode-execute loop:

```
while(start + n > cycles && !illegalOpcode)
{
	// fetch
	opcode = Read(pc++);
	
	// decode
	instr = InstrTable[opcode];
		
	// execute
	Exec(instr);
}
```

The next instruction (the opcode value) is retrieved from memory. Then it's decoded (i.e. the opcode is used to address the instruction table) and the resulting code block is executed.   


## Public methods ##
 
The emulator comes as a single C++ class with five public methods:

```
wdc65c02(BusRead r, BusWrite w);
void NMI();
void IRQ();
void Reset();
void Run(uint32_t n);
```


```wdc65c02(BusRead r, BusWrite w);```

it's the class constructor. It requires you to pass two external functions:

```
uint8_t MemoryRead(uint16_t address);
void MemoryWrite(uint16_t address, uint8_t value);
```

respectively to read/write from/to a memory location (16 bit address, 8 bit value). In such functions you can define your address decoding logic (if any) to address memory mapped I/O, external virtual devices and such.

```
void NMI();
```

triggers a Non-Mascherable Interrupt request, as done by the external pin of the real chip

```
void IRQ();
```

triggers an Interrupt ReQuest, as done by the external pin of the real chip

```
void Reset();
```

performs an hardware reset, as done by the external pin of the real chip

```
void Run(uint32_t n);
```

It runs the CPU for the next 'n' machine instructions.

## Links ##

Some useful stuff I used...

http://en.wikipedia.org/wiki/MOS_Technology_6502

http://www.6502.org/documents/datasheets/mos/

http://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf

http://rubbermallet.org/fake6502.c

## Links i added ##

http://6502.org/tutorials/65c02opcodes.html#8

https://www.westerndesigncenter.com/wdc/documentation/w65c02s.pdf



