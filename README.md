## W65C02S Emulator in C++ ##
Based on https://github.com/gianlucag/mos6502 to be used on my 6502 Emulator on the Casio FX-CG50 :) \
soon...

## Emulator features ##

### Addressing Modes ###

```
// Addressing modes (Arranged according to datasheet)
uint16_t Addr_ABSOL(); // ABSOLUTE
uint16_t Addr_ABIXN(); // ABSOLUTE INDEXED-X INDIRECT
uint16_t Addr_ABSIX(); // ABSOLUTE INDEXED-X
uint16_t Addr_ABSIY(); // ABSOLUTE INDEXED-Y
uint16_t Addr_ABSIN(); // ABSOLUTE INDIRECT
uint16_t Addr_ACCUM(); // ACCUMULATOR
uint16_t Addr_IMMED(); // IMMEDIATE
uint16_t Addr_IMPLI(); // IMPLIED
uint16_t Addr_RELAT(); // RELATIVE
uint16_t Addr_ZEROP(); // ZERO PAGE
uint16_t Addr_ZPIXN(); // ZERO PAGE INDEXED-X INDIRECT
uint16_t Addr_ZRPIX(); // ZERO PAGE INDEXED-X
uint16_t Addr_ZRPIY(); // ZERO PAGE INDEXED-Y
uint16_t Addr_ZRPIN(); // ZERO PAGE INDIRECT
uint16_t Addr_ZPINY(); // ZERO PAGE INDIRECT INDEXED-Y
```

## Inner main loop ##

It's a classic fetch-decode-execute loop:

```
while(cyclesRemaining > 0 && !STOP)
{
	// fetch
	opcode = Read(pc++);

	// decode
	instr = InstrTable[opcode];

	// execute
	Exec(instr);
	cycleCount += instr.cycles;
	cyclesRemaining -=
		cycleMethod == CYCLE_COUNT        ? instr.cycles
		/* cycleMethod == INST_COUNT */   : 1;
}
```

The next instruction (the opcode value) is retrieved from memory. Then it's decoded (i.e. the opcode is used to address the instruction table) and the resulting code block is executed.

### STOP (STP and WAI) ###
- When WAI is executed the second bit of STOP is set
- When STP is executed the first bit of STOP is set
- When STOP is non-zero the emulator won't proceed
- IRQ, NMI and RESET will clear the first bit
- RESET clears the second bit


## Public methods ##
 
The emulator comes as a single C++ class with five public methods:

```
wdc65c02(BusRead r, BusWrite w);
void NMI();
void IRQ();
void Reset();
void Run(
	int32_t cycles,
	uint64_t& cycleCount,
	CycleMethod cycleMethod = CYCLE_COUNT);

uint16_t GetPC();
uint8_t GetS();
uint8_t GetP();
uint8_t GetA();
uint8_t GetX();
uint8_t GetY();
uint8_t GetSTOP();

void SetPC(uint16_t address);
void SetS(uint8_t value);
void SetP(uint8_t value);
void SetA(uint8_t value);
void SetX(uint8_t value);
void SetY(uint8_t value);

void SetResetS(uint8_t value);
void SetResetP(uint8_t value);
void SetResetA(uint8_t value);
void SetResetX(uint8_t value);
void SetResetY(uint8_t value);
uint8_t GetResetS();
uint8_t GetResetP();
uint8_t GetResetA();
uint8_t GetResetX();
uint8_t GetResetY();
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

triggers a Non-Maskable Interrupt request, as done by the external pin of the real chip

```
void IRQ();
```

triggers an Interrupt ReQuest, as done by the external pin of the real chip

```
void Reset();
```

performs a hardware reset, as done by the external pin of the real chip

```
void Run(
	int32_t cycles,
	uint64_t& cycleCount,
	CycleMethod cycleMethod = CYCLE_COUNT);
```

runs the CPU for the next 'n' machine instructions.

## Links ##

Some useful stuff I used...

http://en.wikipedia.org/wiki/MOS_Technology_6502

http://www.6502.org/documents/datasheets/mos/

http://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf

http://rubbermallet.org/fake6502.c

http://6502.org/tutorials/65c02opcodes.html#8

https://www.westerndesigncenter.com/wdc/documentation/w65c02s.pdf



