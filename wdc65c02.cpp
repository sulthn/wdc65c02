#include "wdc65c02.h"

#define NEGATIVE  0x80
#define OVERFLOW  0x40
#define CONSTANT  0x20
#define BREAK     0x10
#define DECIMAL   0x08
#define INTERRUPT 0x04
#define ZERO      0x02
#define CARRY     0x01

#define SET_NEGATIVE(x) (x ? (status |= NEGATIVE) : (status &= (~NEGATIVE)) )
#define SET_OVERFLOW(x) (x ? (status |= OVERFLOW) : (status &= (~OVERFLOW)) )
//#define SET_CONSTANT(x) (x ? (status |= CONSTANT) : (status &= (~CONSTANT)) )
//#define SET_BREAK(x) (x ? (status |= BREAK) : (status &= (~BREAK)) )
#define SET_DECIMAL(x) (x ? (status |= DECIMAL) : (status &= (~DECIMAL)) )
#define SET_INTERRUPT(x) (x ? (status |= INTERRUPT) : (status &= (~INTERRUPT)) )
#define SET_ZERO(x) (x ? (status |= ZERO) : (status &= (~ZERO)) )
#define SET_CARRY(x) (x ? (status |= CARRY) : (status &= (~CARRY)) )

#define IF_NEGATIVE() ((status & NEGATIVE) ? true : false)
#define IF_OVERFLOW() ((status & OVERFLOW) ? true : false)
#define IF_CONSTANT() ((status & CONSTANT) ? true : false)
#define IF_BREAK() ((status & BREAK) ? true : false)
#define IF_DECIMAL() ((status & DECIMAL) ? true : false)
#define IF_INTERRUPT() ((status & INTERRUPT) ? true : false)
#define IF_ZERO() ((status & ZERO) ? true : false)
#define IF_CARRY() ((status & CARRY) ? true : false)

wdc65c02::Instr wdc65c02::InstrTable[256];

wdc65c02::wdc65c02(BusRead r, BusWrite w)
	: reset_A(0x00)
    , reset_X(0x00)
    , reset_Y(0x00)
    , reset_sp(0xFD)
    , reset_status(CONSTANT)
	, STOP(0x00)
{
	Write = (BusWrite)w;
	Read = (BusRead)r;

	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	Instr instr;
	// fill jump table with NOPs
	// 0x_3 and 0x_B
	instr.code = &wdc65c02::Op_NOP;
	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.cycles = 1;
	for(int i = 0; i < 256; i++)
	{
		InstrTable[i] = instr;
	}

	// 0x_2
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.cycles = 2;
	for (int i = 0; i < 0x10; i++) 
	{
		InstrTable[(i << 4) + 0x02] = instr;
	}

	// 0x_4
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.cycles = 4;
	for (int i = 0; i < 0x10; i++) 
	{
		InstrTable[(i << 4) + 0x04] = instr;
	}

	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.cycles = 3;
	InstrTable[0x44] = instr;

	// 0x_C
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.cycles = 4;
	for (int i = 0; i < 0x10; i++) 
	{
		InstrTable[(i << 4) + 0x0c] = instr;
	}

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.cycles = 8;
	InstrTable[0x5C] = instr;


	// insert opcodes

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 4;
	InstrTable[0x6D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 4;
	InstrTable[0x7D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 4;
	InstrTable[0x79] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 2;
	InstrTable[0x69] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 3;
	InstrTable[0x65] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 6;
	InstrTable[0x61] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 4;
	InstrTable[0x75] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 5;
	InstrTable[0x72] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_ADC;
	instr.cycles = 6;
	InstrTable[0x71] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 4;
	InstrTable[0x2D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 4;
	InstrTable[0x3D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 4;
	InstrTable[0x39] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 2;
	InstrTable[0x29] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 3;
	InstrTable[0x25] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 6;
	InstrTable[0x21] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 4;
	InstrTable[0x35] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 5;
	InstrTable[0x32] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_AND;
	instr.cycles = 5;
	InstrTable[0x31] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_ASL;
	instr.cycles = 6;
	InstrTable[0x0E] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_ASL;
	instr.cycles = 7;
	InstrTable[0x1E] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_ASL_ACC;
	instr.cycles = 2;
	InstrTable[0x0A] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_ASL;
	instr.cycles = 5;
	InstrTable[0x06] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_ASL;
	instr.cycles = 6;
	InstrTable[0x16] = instr;

	// BBR (NEW INSTRUCTION)
	instr.addr = &wdc65c02::Addr_ZEROP; // ZERO PAGE, RELATIVE
	instr.cycles = 4;
	instr.code = &wdc65c02::Op_BBR0;
	InstrTable[0x0F] = instr;
	instr.code = &wdc65c02::Op_BBR1;
	InstrTable[0x1F] = instr;
	instr.code = &wdc65c02::Op_BBR2;
	InstrTable[0x2F] = instr;
	instr.code = &wdc65c02::Op_BBR3;
	InstrTable[0x3F] = instr;
	instr.code = &wdc65c02::Op_BBR4;
	InstrTable[0x4F] = instr;
	instr.code = &wdc65c02::Op_BBR5;
	InstrTable[0x5F] = instr;
	instr.code = &wdc65c02::Op_BBR6;
	InstrTable[0x6F] = instr;
	instr.code = &wdc65c02::Op_BBR7;
	InstrTable[0x7F] = instr;

	// BBS (NEW INSTRUCTION)
	instr.code = &wdc65c02::Op_BBS0;
	InstrTable[0x8F] = instr;
	instr.code = &wdc65c02::Op_BBS1;
	InstrTable[0x9F] = instr;
	instr.code = &wdc65c02::Op_BBS2;
	InstrTable[0xAF] = instr;
	instr.code = &wdc65c02::Op_BBS3;
	InstrTable[0xBF] = instr;
	instr.code = &wdc65c02::Op_BBS4;
	InstrTable[0xCf] = instr;
	instr.code = &wdc65c02::Op_BBS5;
	InstrTable[0xDF] = instr;
	instr.code = &wdc65c02::Op_BBS6;
	InstrTable[0xEF] = instr;
	instr.code = &wdc65c02::Op_BBS7;
	InstrTable[0xFF] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BCC;
	instr.cycles = 2;
	InstrTable[0x90] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BCS;
	instr.cycles = 2;
	InstrTable[0xB0] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BEQ;
	instr.cycles = 2;
	InstrTable[0xF0] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_BIT;
	instr.cycles = 4;
	InstrTable[0x2C] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_BIT;
	instr.cycles = 4;
	InstrTable[0x3C] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_BIT_IMMED;
	instr.cycles = 2;
	InstrTable[0x89] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_BIT;
	instr.cycles = 3;
	InstrTable[0x24] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_BIT;
	instr.cycles = 4;
	InstrTable[0x34] = instr; // New adressing mode

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BMI;
	instr.cycles = 2;
	InstrTable[0x30] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BNE;
	instr.cycles = 2;
	InstrTable[0xD0] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BPL;
	instr.cycles = 2;
	InstrTable[0x10] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BRA;  // NEW INSTRUCTION
	instr.cycles = 3;
	InstrTable[0x80] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_BRK;
	instr.cycles = 7;
	InstrTable[0x00] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BVC;
	instr.cycles = 2;
	InstrTable[0x50] = instr;

	instr.addr = &wdc65c02::Addr_RELAT;
	instr.code = &wdc65c02::Op_BVS;
	instr.cycles = 2;
	InstrTable[0x70] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_CLC;
	instr.cycles = 2;
	InstrTable[0x18] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_CLD;
	instr.cycles = 2;
	InstrTable[0xD8] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_CLI;
	instr.cycles = 2;
	InstrTable[0x58] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_CLV;
	instr.cycles = 2;
	InstrTable[0xB8] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 4;
	InstrTable[0xCD] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 4;
	InstrTable[0xDD] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 4;
	InstrTable[0xD9] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 2;
	InstrTable[0xC9] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 3;
	InstrTable[0xC5] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 6;
	InstrTable[0xC1] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 4;
	InstrTable[0xD5] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 5;
	InstrTable[0xD2] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_CMP;
	instr.cycles = 3;
	InstrTable[0xD1] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_CPX;
	instr.cycles = 4;
	InstrTable[0xEC] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_CPX;
	instr.cycles = 2;
	InstrTable[0xE0] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_CPX;
	instr.cycles = 3;
	InstrTable[0xE4] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_CPY;
	instr.cycles = 4;
	InstrTable[0xCC] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_CPY;
	instr.cycles = 2;
	InstrTable[0xC0] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_CPY;
	instr.cycles = 3;
	InstrTable[0xC4] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_DEC;
	instr.cycles = 6;
	InstrTable[0xCE] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_DEC;
	instr.cycles = 7;
	InstrTable[0xDE] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_DEC_ACC;
	instr.cycles = 2;
	InstrTable[0x3A] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_DEC;
	instr.cycles = 5;
	InstrTable[0xC6] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_DEC;
	instr.cycles = 6;
	InstrTable[0xD6] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_DEX;
	instr.cycles = 2;
	InstrTable[0xCA] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_DEY;
	instr.cycles = 2;
	InstrTable[0x88] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 4;
	InstrTable[0x4D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 4;
	InstrTable[0x5D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 4;
	InstrTable[0x59] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 2;
	InstrTable[0x49] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 3;
	InstrTable[0x45] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 6;
	InstrTable[0x41] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 4;
	InstrTable[0x55] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 5;
	InstrTable[0x52] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_EOR;
	instr.cycles = 5;
	InstrTable[0x51] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_INC;
	instr.cycles = 6;
	InstrTable[0xEE] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_INC;
	instr.cycles = 7;
	InstrTable[0xFE] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_INC_ACC;
	instr.cycles = 2;
	InstrTable[0x1A] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_INC;
	instr.cycles = 5;
	InstrTable[0xE6] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_INC;
	instr.cycles = 6;
	InstrTable[0xF6] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_INX;
	instr.cycles = 2;
	InstrTable[0xE8] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_INY;
	instr.cycles = 2;
	InstrTable[0xC8] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_JMP;
	instr.cycles = 3;
	InstrTable[0x4C] = instr;
	instr.addr = &wdc65c02::Addr_ABIXN;
	instr.code = &wdc65c02::Op_JMP;
	instr.cycles = 6;
	InstrTable[0x7C] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ABSIN;
	instr.code = &wdc65c02::Op_JMP;
	instr.cycles = 6;
	InstrTable[0x6C] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_JSR;
	instr.cycles = 6;
	InstrTable[0x20] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 4;
	InstrTable[0xAD] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 4;
	InstrTable[0xBD] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 4;
	InstrTable[0xB9] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 2;
	InstrTable[0xA9] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 3;
	InstrTable[0xA5] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 6;
	InstrTable[0xA1] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 4;
	InstrTable[0xB5] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 5;
	InstrTable[0xB2] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_LDA;
	instr.cycles = 5;
	InstrTable[0xB1] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_LDX;
	instr.cycles = 4;
	InstrTable[0xAE] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_LDX;
	instr.cycles = 4;
	InstrTable[0xBE] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_LDX;
	instr.cycles = 2;
	InstrTable[0xA2] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_LDX;
	instr.cycles = 3;
	InstrTable[0xA6] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIY;
	instr.code = &wdc65c02::Op_LDX;
	instr.cycles = 4;
	InstrTable[0xB6] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_LDY;
	instr.cycles = 4;
	InstrTable[0xAC] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_LDY;
	instr.cycles = 4;
	InstrTable[0xBC] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_LDY;
	instr.cycles = 2;
	InstrTable[0xA0] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_LDY;
	instr.cycles = 3;
	InstrTable[0xA4] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_LDY;
	instr.cycles = 4;
	InstrTable[0xB4] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_LSR;
	instr.cycles = 6;
	InstrTable[0x4E] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_LSR;
	instr.cycles = 7;
	InstrTable[0x5E] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_LSR_ACC;
	instr.cycles = 2;
	InstrTable[0x4A] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_LSR;
	instr.cycles = 5;
	InstrTable[0x46] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_LSR;
	instr.cycles = 6;
	InstrTable[0x56] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_NOP;
	instr.cycles = 2;
	InstrTable[0xEA] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 4;
	InstrTable[0x0D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 4;
	InstrTable[0x1D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 4;
	InstrTable[0x19] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 2;
	InstrTable[0x09] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 3;
	InstrTable[0x05] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 6;
	InstrTable[0x01] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 4;
	InstrTable[0x15] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 5;
	InstrTable[0x12] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_ORA;
	instr.cycles = 5;
	InstrTable[0x11] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PHA;
	instr.cycles = 3;
	InstrTable[0x48] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PHP;
	instr.cycles = 3;
	InstrTable[0x08] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PHX;
	instr.cycles = 3;
	InstrTable[0xDA] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PHY;
	instr.cycles = 3;
	InstrTable[0x5A] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PLA;
	instr.cycles = 4;
	InstrTable[0x68] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PLP;
	instr.cycles = 4;
	InstrTable[0x28] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PLX;  // NEW INSTRUCTION
	instr.cycles = 4;
	InstrTable[0xFA] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_PLY;  // NEW INSTRUCTION
	instr.cycles = 4;
	InstrTable[0x7A] = instr;

	// RMB (NEW INSTRUCTION)
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.cycles = 5;
	instr.code = &wdc65c02::Op_RMB0;
	InstrTable[0x07] = instr;
	instr.code = &wdc65c02::Op_RMB1;
	InstrTable[0x17] = instr;
	instr.code = &wdc65c02::Op_RMB2;
	InstrTable[0x27] = instr;
	instr.code = &wdc65c02::Op_RMB3;
	InstrTable[0x37] = instr;
	instr.code = &wdc65c02::Op_RMB4;
	InstrTable[0x47] = instr;
	instr.code = &wdc65c02::Op_RMB5;
	InstrTable[0x57] = instr;
	instr.code = &wdc65c02::Op_RMB6;
	InstrTable[0x67] = instr;
	instr.code = &wdc65c02::Op_RMB7;
	InstrTable[0x77] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_ROL;
	instr.cycles = 6;
	InstrTable[0x2E] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_ROL;
	instr.cycles = 7;
	InstrTable[0x3E] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_ROL_ACC;
	instr.cycles = 2;
	InstrTable[0x2A] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_ROL;
	instr.cycles = 5;
	InstrTable[0x26] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_ROL;
	instr.cycles = 6;
	InstrTable[0x36] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_ROR;
	instr.cycles = 6;
	InstrTable[0x6E] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_ROR;
	instr.cycles = 7;
	InstrTable[0x7E] = instr;
	instr.addr = &wdc65c02::Addr_ACCUM;
	instr.code = &wdc65c02::Op_ROR_ACC;
	instr.cycles = 2;
	InstrTable[0x6A] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_ROR;
	instr.cycles = 5;
	InstrTable[0x66] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_ROR;
	instr.cycles = 6;
	InstrTable[0x76] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_RTI;
	instr.cycles = 6;
	InstrTable[0x40] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_RTS;
	instr.cycles = 6;
	InstrTable[0x60] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 4;
	InstrTable[0xED] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 4;
	InstrTable[0xFD] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 4;
	InstrTable[0xF9] = instr;
	instr.addr = &wdc65c02::Addr_IMMED;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 2;
	InstrTable[0xE9] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 3;
	InstrTable[0xE5] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 6;
	InstrTable[0xE1] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 4;
	InstrTable[0xF5] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 5;
	InstrTable[0xF2] = instr;
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_SBC;
	instr.cycles = 5;
	InstrTable[0xF1] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_SEC;
	instr.cycles = 2;
	InstrTable[0x38] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_SED;
	instr.cycles = 2;
	InstrTable[0xF8] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_SEI;
	instr.cycles = 2;
	InstrTable[0x78] = instr;

	// SMB (NEW INSTRUCTION)
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.cycles = 5;
	instr.code = &wdc65c02::Op_SMB0;
	InstrTable[0x87] = instr;
	instr.code = &wdc65c02::Op_SMB1;
	InstrTable[0x97] = instr;
	instr.code = &wdc65c02::Op_SMB2;
	InstrTable[0xA7] = instr;
	instr.code = &wdc65c02::Op_SMB3;
	InstrTable[0xB7] = instr;
	instr.code = &wdc65c02::Op_SMB4;
	InstrTable[0xC7] = instr;
	instr.code = &wdc65c02::Op_SMB5;
	InstrTable[0xD7] = instr;
	instr.code = &wdc65c02::Op_SMB6;
	InstrTable[0xE7] = instr;
	instr.code = &wdc65c02::Op_SMB7;
	InstrTable[0xF7] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 4;
	InstrTable[0x8D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 6;
	InstrTable[0x9D] = instr;
	instr.addr = &wdc65c02::Addr_ABSIY;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 5;
	InstrTable[0x99] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 3;
	InstrTable[0x85] = instr;
	instr.addr = &wdc65c02::Addr_ZPIXN;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 6;
	InstrTable[0x81] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 4;
	InstrTable[0x95] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIN;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 6;
	InstrTable[0x92] = instr; // New adressing mode
	instr.addr = &wdc65c02::Addr_ZPINY;
	instr.code = &wdc65c02::Op_STA;
	instr.cycles = 6;
	InstrTable[0x91] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_STP;  // NEW INSTRUCTION
	instr.cycles = 2;
	InstrTable[0xDB] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_STX;
	instr.cycles = 4;
	InstrTable[0x8E] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_STX;
	instr.cycles = 3;
	InstrTable[0x86] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIY;
	instr.code = &wdc65c02::Op_STX;
	instr.cycles = 4;
	InstrTable[0x96] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_STY;
	instr.cycles = 4;
	InstrTable[0x8C] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_STY;
	instr.cycles = 3;
	InstrTable[0x84] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_STY;
	instr.cycles = 4;
	InstrTable[0x94] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_STZ;  // NEW INSTRUCTION
	instr.cycles = 5;
	InstrTable[0x9C] = instr;
	instr.addr = &wdc65c02::Addr_ABSIX;
	instr.code = &wdc65c02::Op_STZ;
	instr.cycles = 6;
	InstrTable[0x9E] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_STZ;
	instr.cycles = 4;
	InstrTable[0x64] = instr;
	instr.addr = &wdc65c02::Addr_ZRPIX;
	instr.code = &wdc65c02::Op_STZ;
	instr.cycles = 5;
	InstrTable[0x74] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TAX;
	instr.cycles = 2;
	InstrTable[0xAA] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TAY;
	instr.cycles = 2;
	InstrTable[0xA8] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_TRB;  // NEW INSTRUCTION
	instr.cycles = 6;
	InstrTable[0x1C] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_TRB;
	instr.cycles = 5;
	InstrTable[0x14] = instr;

	instr.addr = &wdc65c02::Addr_ABSOL;
	instr.code = &wdc65c02::Op_TSB;  // NEW INSTRUCTION
	instr.cycles = 6;
	InstrTable[0x0C] = instr;
	instr.addr = &wdc65c02::Addr_ZEROP;
	instr.code = &wdc65c02::Op_TSB;
	instr.cycles = 5;
	InstrTable[0x04] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TSX;
	instr.cycles = 2;
	InstrTable[0xBA] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TXA;
	instr.cycles = 2;
	InstrTable[0x8A] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TXS;
	instr.cycles = 2;
	InstrTable[0x9A] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_TYA;
	instr.cycles = 2;
	InstrTable[0x98] = instr;

	instr.addr = &wdc65c02::Addr_IMPLI;
	instr.code = &wdc65c02::Op_WAI;  // NEW INSTRUCTION
	instr.cycles = 5;
	InstrTable[0xCB] = instr;

	return;
}


// INTERNAL

void wdc65c02::Reset()
{
	STOP = 0;

	A = reset_A;
	Y = reset_Y;
	X = reset_X;

	// load PC from reset vector
	uint8_t pcl = Read(rstVectorL);
	uint8_t pch = Read(rstVectorH);
	pc = (pch << 8) + pcl;

	sp = reset_sp;

	status = reset_status | CONSTANT | BREAK;

	return;
}

void wdc65c02::StackPush(uint8_t byte)
{
	Write(0x0100 + sp, byte);
	if(sp == 0x00) sp = 0xFF;
	else sp--;
}

uint8_t wdc65c02::StackPop()
{
	if(sp == 0xFF) sp = 0x00;
	else sp++;
	return Read(0x0100 + sp);
}

void wdc65c02::IRQ()
{
	if (STOP & 0b01) return;
	if (STOP & 0b10) {
		STOP &= 0b11111101;
		pc++;
	}
	if(!IF_INTERRUPT())
	{
		//SET_BREAK(0);
		StackPush((pc >> 8) & 0xFF);
		StackPush(pc & 0xFF);
		StackPush((status & ~BREAK) | CONSTANT);
		SET_INTERRUPT(1);
		SET_DECIMAL(0);

		// load PC from irq vector
		uint8_t pcl = Read(irqVectorL);
		uint8_t pch = Read(irqVectorH);
		pc = (pch << 8) + pcl;
	}
	return;
}

void wdc65c02::NMI()
{
	if (STOP & 0b01) return;
	if (STOP & 0b10) {
		STOP &= 0b11111101;
		pc++;
	}
	//SET_BREAK(0);
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
	StackPush((status & ~BREAK) | CONSTANT);
	SET_INTERRUPT(1);
	SET_DECIMAL(0);

	// load PC from NMI vector
	uint8_t pcl = Read(nmiVectorL);
	uint8_t pch = Read(nmiVectorH);
	pc = (pch << 8) + pcl;
	return;
}

void wdc65c02::Run(
	int32_t cyclesRemaining,
	uint64_t& cycleCount,
	CycleMethod cycleMethod
) {
	uint8_t opcode;
	Instr instr;

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
}

void wdc65c02::Exec(Instr i)
{
	uint16_t src = (this->*i.addr)();
	(this->*i.code)(src);
}

uint16_t wdc65c02::GetPC()
{
    return pc;
}

uint8_t wdc65c02::GetS()
{
    return sp;
}

uint8_t wdc65c02::GetP()
{
    return status;
}

uint8_t wdc65c02::GetA()
{
    return A;
}

uint8_t wdc65c02::GetX()
{
    return X;
}

uint8_t wdc65c02::GetY()
{
    return Y;
}

uint8_t wdc65c02::GetSTOP()
{
    return STOP;
}

void wdc65c02::SetPC(uint16_t address) {
	pc = address;
}

void wdc65c02::SetS(uint8_t value) {
	sp = value;
}

void wdc65c02::SetP(uint8_t value) {
	status = value | CONSTANT | BREAK;
}

void wdc65c02::SetA(uint8_t value) {
	A = value;
}

void wdc65c02::SetX(uint8_t value) {
	X = value;
}

void wdc65c02::SetY(uint8_t value) {
	Y = value;
}

void wdc65c02::SetResetS(uint8_t value)
{
    reset_sp = value;
}

void wdc65c02::SetResetP(uint8_t value)
{
    reset_status = value | CONSTANT | BREAK;
}

void wdc65c02::SetResetA(uint8_t value)
{
    reset_A = value;
}

void wdc65c02::SetResetX(uint8_t value)
{
    reset_X = value;
}

void wdc65c02::SetResetY(uint8_t value)
{
    reset_Y = value;
}

uint8_t wdc65c02::GetResetS()
{
    return reset_sp;
}

uint8_t wdc65c02::GetResetP()
{
    return reset_status;
}

uint8_t wdc65c02::GetResetA()
{
    return reset_A;
}

uint8_t wdc65c02::GetResetX()
{
    return reset_X;
}

uint8_t wdc65c02::GetResetY()
{
    return reset_Y;
}


// ADDRESSING MODES

uint16_t wdc65c02::Addr_ABSOL()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t addr;

	addrL = Read(pc++);
	addrH = Read(pc++);

	addr = addrL + (addrH << 8);

	return addr;
}

uint16_t wdc65c02::Addr_ABIXN()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t effL;
	uint16_t effH;
	uint16_t abs;
	uint16_t addr;

	addrL = Read(pc++);
	addrH = Read(pc++);
	abs = ((addrH << 8) | addrL) + X;

	effL = Read(abs);
	effH = Read(abs + 1);
	addr = effL + (effH << 8);
	return addr;
}

uint16_t wdc65c02::Addr_ABSIX()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;

	addrL = Read(pc++);
	addrH = Read(pc++);

	addr = addrL + (addrH << 8) + X;
	return addr;
}

uint16_t wdc65c02::Addr_ABSIY()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;

	addrL = Read(pc++);
	addrH = Read(pc++);

	addr = addrL + (addrH << 8) + Y;
	return addr;
}

uint16_t wdc65c02::Addr_ABSIN()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t effL;
	uint16_t effH;
	uint16_t abs;
	uint16_t addr;

	addrL = Read(pc++);
	addrH = Read(pc++);
	abs = (addrH << 8) | addrL;

	effL = Read(abs);
	// The 6502 has a bug where the page address doesn't increment. The W65C02S fixes that.
	effH = Read(abs + 1);
	addr = effL + (effH << 8);
	return addr;
}

uint16_t wdc65c02::Addr_ACCUM()
{
	return 0; // not used
}

uint16_t wdc65c02::Addr_IMMED()
{
	return pc++;
}

uint16_t wdc65c02::Addr_IMPLI()
{
	return 0; // not used
}

uint16_t wdc65c02::Addr_RELAT()
{
	uint16_t offset;
	uint16_t addr;

	offset = (uint16_t)Read(pc++);
	if (offset & 0x80) offset |= 0xFF00;
	addr = pc + (int16_t)offset;
	return addr;
}

uint16_t wdc65c02::Addr_ZEROP()
{
	return Read(pc++);
}

uint16_t wdc65c02::Addr_ZPIXN()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;

	zeroL = (Read(pc++) + X) & 0xFF;
	zeroH = (zeroL + 1) & 0xFF;
	addr = Read(zeroL) + (Read(zeroH) << 8);

	return addr;
}

uint16_t wdc65c02::Addr_ZRPIX()
{
	uint16_t addr = (Read(pc++) + X) & 0xFF;
	return addr;
}

uint16_t wdc65c02::Addr_ZRPIY()
{
	uint16_t addr = (Read(pc++) + Y) & 0xFF;
	return addr;
}

uint16_t wdc65c02::Addr_ZRPIN()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;

	zeroL = Read(pc++);
	zeroH = (zeroL + 1) & 0xFF;
	addr = Read(zeroL) + (Read(zeroH) << 8);
	
	return addr;
}

uint16_t wdc65c02::Addr_ZPINY()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;

	zeroL = Read(pc++);
	zeroH = (zeroL + 1) & 0xFF;
	addr = Read(zeroL) + (Read(zeroH) << 8) + Y;

	return addr;
}


// Operations

void wdc65c02::Op_ADC(uint16_t src)
{
	uint8_t m = Read(src);
	unsigned int tmp = m + A + (IF_CARRY() ? 1 : 0);
	SET_ZERO(!(tmp & 0xFF));
	if (IF_DECIMAL())
	{
		if (((A & 0xF) + (m & 0xF) + (IF_CARRY() ? 1 : 0)) > 9) tmp += 6;
		SET_OVERFLOW(!((A ^ m) & 0x80) && ((A ^ tmp) & 0x80));
		if (tmp > 0x99)
		{
			tmp += 96;
		}
		SET_ZERO(!(tmp & 0xFF));
		SET_NEGATIVE(tmp & 0x80);
		SET_CARRY(tmp > 0x99);
	}
	else
	{
		SET_NEGATIVE(tmp & 0x80);
		SET_OVERFLOW(!((A ^ m) & 0x80) && ((A ^ tmp) & 0x80));
		SET_CARRY(tmp > 0xFF);
	}

	A = tmp & 0xFF;
	return;
}

void wdc65c02::Op_AND(uint16_t src)
{
	uint8_t m = Read(src);
	uint8_t res = m & A;
	SET_NEGATIVE(res & 0x80);
	SET_ZERO(!res);
	A = res;
	return;
}

void wdc65c02::Op_ASL(uint16_t src)
{
	uint8_t m = Read(src);
	SET_CARRY(m & 0x80);
	m <<= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Write(src, m);
	return;
}

void wdc65c02::Op_ASL_ACC(uint16_t src)
{
	uint8_t m = A;
	SET_CARRY(m & 0x80);
	m <<= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}


// BRANCH ON BIT RESET INSTRUCTIONS

void wdc65c02::Op_BBR0(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00000001)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR1(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00000010)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR2(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00000100)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR3(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00001000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR4(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00010000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR5(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b00100000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR6(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b01000000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBR7(uint16_t src)
{
	uint16_t offset;

	if (~Read(src) & 0b10000000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}


// BRANCH ON BIT SET INSTRUCTIONS

void wdc65c02::Op_BBS0(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00000001)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS1(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00000010)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS2(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00000100)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS3(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00001000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS4(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00010000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS5(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b00100000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS6(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b01000000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}

void wdc65c02::Op_BBS7(uint16_t src)
{
	uint16_t offset;

	if (Read(src) & 0b10000000)
	{
		offset = (uint16_t)Read(pc++);
		if (offset & 0x80) offset |= 0xFF00;
		pc += (int16_t)offset;  // RELATIVE
	} else {
		pc++;
	}
	return;
}


void wdc65c02::Op_BCC(uint16_t src)
{
	if (!IF_CARRY())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BCS(uint16_t src)
{
	if (IF_CARRY())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BEQ(uint16_t src)
{
	if (IF_ZERO())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BIT(uint16_t src)
{
	uint8_t m = Read(src);
	uint8_t res = m & A;
	SET_NEGATIVE(res & 0x80);
	status = (status & 0x3F) | (uint8_t)(m & 0xC0) | CONSTANT | BREAK;
	SET_ZERO(!res);
	return;
}

void wdc65c02::Op_BIT_IMMED(uint16_t src) {
	uint8_t m = Read(src);
	uint8_t res = m & A;
	SET_ZERO(!res);
	return;
}

void wdc65c02::Op_BMI(uint16_t src)
{
	if (IF_NEGATIVE())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BNE(uint16_t src)
{
	if (!IF_ZERO())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BPL(uint16_t src)
{
	if (!IF_NEGATIVE())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BRA(uint16_t src)
{
	if (1 == 1)
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BRK(uint16_t src)
{
	pc++;
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
	StackPush(status | CONSTANT | BREAK);
	SET_INTERRUPT(1);
	SET_DECIMAL(0);
	pc = (Read(irqVectorH) << 8) + Read(irqVectorL);
	return;
}

void wdc65c02::Op_BVC(uint16_t src)
{
	if (!IF_OVERFLOW())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_BVS(uint16_t src)
{
	if (IF_OVERFLOW())
	{
		pc = src;
	}
	return;
}

void wdc65c02::Op_CLC(uint16_t src)
{
	SET_CARRY(0);
	return;
}

void wdc65c02::Op_CLD(uint16_t src)
{
	SET_DECIMAL(0);
	return;
}

void wdc65c02::Op_CLI(uint16_t src)
{
	SET_INTERRUPT(0);
	return;
}

void wdc65c02::Op_CLV(uint16_t src)
{
	SET_OVERFLOW(0);
	return;
}

void wdc65c02::Op_CMP(uint16_t src)
{
	unsigned int tmp = A - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}

void wdc65c02::Op_CPX(uint16_t src)
{
	unsigned int tmp = X - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}

void wdc65c02::Op_CPY(uint16_t src)
{
	unsigned int tmp = Y - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}

void wdc65c02::Op_DEC(uint16_t src)
{
	uint8_t m = Read(src);
	m = (m - 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Write(src, m);
	return;
}

void wdc65c02::Op_DEC_ACC(uint16_t src)
{
	uint8_t m = A;
	m = (m - 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}

void wdc65c02::Op_DEX(uint16_t src)
{
	uint8_t m = X;
	m = (m - 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	X = m;
	return;
}

void wdc65c02::Op_DEY(uint16_t src)
{
	uint8_t m = Y;
	m = (m - 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Y = m;
	return;
}

void wdc65c02::Op_EOR(uint16_t src)
{
	uint8_t m = Read(src);
	m = A ^ m;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
}

void wdc65c02::Op_INC(uint16_t src)
{
	uint8_t m = Read(src);
	m = (m + 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Write(src, m);
}

void wdc65c02::Op_INC_ACC(uint16_t src)
{
	uint8_t m = A;
	m = (m + 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
}

void wdc65c02::Op_INX(uint16_t src)
{
	uint8_t m = X;
	m = (m + 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	X = m;
}

void wdc65c02::Op_INY(uint16_t src)
{
	uint8_t m = Y;
	m = (m + 1) & 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Y = m;
}

void wdc65c02::Op_JMP(uint16_t src)
{
	pc = src;
}

void wdc65c02::Op_JSR(uint16_t src)
{
	pc--;
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
	pc = src;
}

void wdc65c02::Op_LDA(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
}

void wdc65c02::Op_LDX(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	X = m;
}

void wdc65c02::Op_LDY(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Y = m;
}

void wdc65c02::Op_LSR(uint16_t src)
{
	uint8_t m = Read(src);
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	Write(src, m);
}

void wdc65c02::Op_LSR_ACC(uint16_t src)
{
	uint8_t m = A;
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	A = m;
}

void wdc65c02::Op_NOP(uint16_t src)
{
	return;
}

void wdc65c02::Op_ORA(uint16_t src)
{
	uint8_t m = Read(src);
	m = A | m;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
}

void wdc65c02::Op_PHA(uint16_t src)
{
	StackPush(A);
	return;
}

void wdc65c02::Op_PHP(uint16_t src)
{
	StackPush(status | CONSTANT | BREAK);
	return;
}

void wdc65c02::Op_PHX(uint16_t src)
{
	StackPush(X);
	return;
}

void wdc65c02::Op_PHY(uint16_t src)
{
	StackPush(Y);
	return;
}

void wdc65c02::Op_PLA(uint16_t src)
{
	A = StackPop();
	SET_NEGATIVE(A & 0x80);
	SET_ZERO(!A);
	return;
}

void wdc65c02::Op_PLP(uint16_t src)
{
	status = StackPop() | CONSTANT | BREAK;
	//SET_CONSTANT(1);
	return;
}

void wdc65c02::Op_PLX(uint16_t src)
{
	X = StackPop();
	SET_NEGATIVE(X & 0x80);
	SET_ZERO(!X);
	return;
}

void wdc65c02::Op_PLY(uint16_t src)
{
	Y = StackPop();
	SET_NEGATIVE(Y & 0x80);
	SET_ZERO(!Y);
	return;
}

// RESET MEMORY BIT INSTRUCTIONS

void wdc65c02::Op_RMB0(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11111110;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB1(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11111101;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB2(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11111011;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB3(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11110111;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB4(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11101111;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB5(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b11011111;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB6(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b10111111;
	Write(src, m);

	return;
}

void wdc65c02::Op_RMB7(uint16_t src)
{
	uint8_t m = Read(src);
	m &= 0b01111111;
	Write(src, m);

	return;
}

void wdc65c02::Op_ROL(uint16_t src)
{
	uint16_t m = Read(src);
	m <<= 1;
	if (IF_CARRY()) m |= 0x01;
	SET_CARRY(m > 0xFF);
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Write(src, m);
	return;
}

void wdc65c02::Op_ROL_ACC(uint16_t src)
{
	uint16_t m = A;
	m <<= 1;
	if (IF_CARRY()) m |= 0x01;
	SET_CARRY(m > 0xFF);
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}

void wdc65c02::Op_ROR(uint16_t src)
{
	uint16_t m = Read(src);
	if (IF_CARRY()) m |= 0x100;
	SET_CARRY(m & 0x01);
	m >>= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Write(src, m);
	return;
}

void wdc65c02::Op_ROR_ACC(uint16_t src)
{
	uint16_t m = A;
	if (IF_CARRY()) m |= 0x100;
	SET_CARRY(m & 0x01);
	m >>= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}

void wdc65c02::Op_RTI(uint16_t src)
{
	uint8_t lo, hi;

	status = StackPop() | CONSTANT | BREAK;

	lo = StackPop();
	hi = StackPop();

	pc = (hi << 8) | lo;
	return;
}

void wdc65c02::Op_RTS(uint16_t src)
{
	uint8_t lo, hi;

	lo = StackPop();
	hi = StackPop();

	pc = ((hi << 8) | lo) + 1;
	return;
}

void wdc65c02::Op_SBC(uint16_t src)
{
	uint8_t m = Read(src);
	unsigned int tmp = A - m - (IF_CARRY() ? 0 : 1);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	SET_OVERFLOW(((A ^ tmp) & 0x80) && ((A ^ m) & 0x80));

	if (IF_DECIMAL())
	{
		if ( ((A & 0x0F) - (IF_CARRY() ? 0 : 1)) < (m & 0x0F)) tmp -= 6;
		if (tmp > 0x99)
		{
			tmp -= 0x60;
		}
		SET_NEGATIVE(tmp & 0x80);
		SET_ZERO(!(tmp & 0xFF));
	}
	SET_CARRY(tmp < 0x100);
	A = (tmp & 0xFF);
	return;
}

void wdc65c02::Op_SEC(uint16_t src)
{
	SET_CARRY(1);
	return;
}

void wdc65c02::Op_SED(uint16_t src)
{
	SET_DECIMAL(1);
	return;
}

void wdc65c02::Op_SEI(uint16_t src)
{
	SET_INTERRUPT(1);
	return;
}

// SET MEMORY BIT INSTRUCTIONS

void wdc65c02::Op_SMB0(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00000001;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB1(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00000010;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB2(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00000100;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB3(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00001000;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB4(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00010000;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB5(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b00100000;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB6(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b01000000;
	Write(src, m);

	return;
}

void wdc65c02::Op_SMB7(uint16_t src)
{
	uint8_t m = Read(src);
	m |= 0b10000000;
	Write(src, m);

	return;
}


void wdc65c02::Op_STA(uint16_t src)
{
	Write(src, A);
	return;
}

void wdc65c02::Op_STP(uint16_t src)
{
	STOP |= 0b00000001;
	pc--;
	return;
}

void wdc65c02::Op_STX(uint16_t src)
{
	Write(src, X);
	return;
}

void wdc65c02::Op_STY(uint16_t src)
{
	Write(src, Y);
	return;
}

void wdc65c02::Op_STZ(uint16_t src)
{
	Write(src, 0);
	return;
}

void wdc65c02::Op_TAX(uint16_t src)
{
	uint8_t m = A;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	X = m;
	return;
}

void wdc65c02::Op_TAY(uint16_t src)
{
	uint8_t m = A;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	Y = m;
	return;
}

void wdc65c02::Op_TRB(uint16_t src)
{
	uint16_t m = Read(src);
	SET_ZERO(!(A & m));
	Write(src, ~A & m);

	return;
}

void wdc65c02::Op_TSB(uint16_t src)
{
	uint16_t m = Read(src);
	SET_ZERO(!(A & m));
	Write(src, A | m);

	return;
}

void wdc65c02::Op_TSX(uint16_t src)
{
	uint8_t m = sp;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	X = m;
	return;
}

void wdc65c02::Op_TXA(uint16_t src)
{
	uint8_t m = X;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}

void wdc65c02::Op_TXS(uint16_t src)
{
	sp = X;
	return;
}

void wdc65c02::Op_TYA(uint16_t src)
{
	uint8_t m = Y;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	A = m;
	return;
}

void wdc65c02::Op_WAI(uint16_t src)
{
	STOP |= 0b00000010;
	pc--;
	return;
}
