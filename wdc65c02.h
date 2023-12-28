#pragma once
#include <stdint.h>
#include <string.h>

class wdc65c02
{
private:
    // register reset values
    uint8_t reset_A;
    uint8_t reset_X;
    uint8_t reset_Y;
    uint8_t reset_sp;
    uint8_t reset_status;

	// registers
	uint8_t A; // accumulator
	uint8_t X; // X-index
	uint8_t Y; // Y-index

	// stack pointer
	uint8_t sp;

	// program counter
	uint16_t pc;

	// status register
	uint8_t status;

	typedef void (wdc65c02::*CodeExec)(uint16_t);
	typedef uint16_t (wdc65c02::*AddrExec)();

	struct Instr
	{
		AddrExec addr;
		CodeExec code;
		uint8_t cycles;
	};

	static Instr InstrTable[256];

	void Exec(Instr i);

	bool illegalOpcode;

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

	// OpCodes (Arranged alphabetically)
	void Op_ADC(uint16_t src);
	void Op_AND(uint16_t src);
	void Op_ASL(uint16_t src); 	void Op_ASL_ACC(uint16_t src);

	void Op_BBR(uint16_t src);  // WDC65C02 INSTRUCTION
	void Op_BBS(uint16_t src);  // WDC65C02 INSTRUCTION
	
	void Op_BCC(uint16_t src);
	void Op_BCS(uint16_t src);

	void Op_BEQ(uint16_t src);
	void Op_BIT(uint16_t src);
	void Op_BMI(uint16_t src);
	void Op_BNE(uint16_t src);
	void Op_BPL(uint16_t src);
	void Op_BRA(uint16_t src);
	void Op_BRK(uint16_t src);

	void Op_BVC(uint16_t src);
	void Op_BVS(uint16_t src);
	void Op_CLC(uint16_t src);
	void Op_CLD(uint16_t src);
	void Op_CLI(uint16_t src);
	void Op_CLV(uint16_t src);
	void Op_CMP(uint16_t src);

	void Op_CPX(uint16_t src);
	void Op_CPY(uint16_t src);
	void Op_DEC(uint16_t src);
	void Op_DEX(uint16_t src);
	void Op_DEY(uint16_t src);
	void Op_EOR(uint16_t src);
	void Op_INC(uint16_t src);

	void Op_INX(uint16_t src);
	void Op_INY(uint16_t src);
	void Op_JMP(uint16_t src);
	void Op_JSR(uint16_t src);
	void Op_LDA(uint16_t src);
	void Op_LDX(uint16_t src);
	void Op_LDY(uint16_t src);

	void Op_LSR(uint16_t src); 	void Op_LSR_ACC(uint16_t src);
	void Op_NOP(uint16_t src);
	void Op_ORA(uint16_t src);
	void Op_PHA(uint16_t src);
	void Op_PHP(uint16_t src);
	void Op_PHX(uint16_t src);
	void Op_PHY(uint16_t src);

	void Op_PLA(uint16_t src);
	void Op_PLP(uint16_t src);
	void Op_PLX(uint16_t src);
	void Op_PLY(uint16_t src);
	void Op_RMB(uint16_t src);
	void Op_ROL(uint16_t src); 	void Op_ROL_ACC(uint16_t src);
	void Op_ROR(uint16_t src);	void Op_ROR_ACC(uint16_t src);
	
	void Op_RTI(uint16_t src);
	void Op_RTS(uint16_t src);
	void Op_SBC(uint16_t src);
	void Op_SEC(uint16_t src);
	void Op_SED(uint16_t src);
	void Op_SEI(uint16_t src);
	void Op_SMB(uint16_t src);

	void Op_STA(uint16_t src);
	void Op_STP(uint16_t src);
	void Op_STX(uint16_t src);
	void Op_STY(uint16_t src);
	void Op_STZ(uint16_t src);
	void Op_TAX(uint16_t src);
	void Op_TAY(uint16_t src);

	void Op_TRB(uint16_t src);
	void Op_TSB(uint16_t src);
	void Op_TSX(uint16_t src);
	void Op_TXA(uint16_t src);
	void Op_TXS(uint16_t src);
	void Op_TYA(uint16_t src);
	void Op_WAI(uint16_t src);

	// IRQ, reset, NMI vectors
	static const uint16_t irqVectorH = 0xFFFF;
	static const uint16_t irqVectorL = 0xFFFE;
	static const uint16_t rstVectorH = 0xFFFD;
	static const uint16_t rstVectorL = 0xFFFC;
	static const uint16_t nmiVectorH = 0xFFFB;
	static const uint16_t nmiVectorL = 0xFFFA;

	// read/write callbacks
	typedef void (*BusWrite)(uint16_t, uint8_t);
	typedef uint8_t (*BusRead)(uint16_t);
	BusRead Read;
	BusWrite Write;

	// stack operations
	inline void StackPush(uint8_t byte);
	inline uint8_t StackPop();

public:
	enum CycleMethod {
		INST_COUNT,
		CYCLE_COUNT,
	};
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
};
