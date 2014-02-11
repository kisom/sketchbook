/*
 * Copyright (c) 2014 Kyle Isom <kyle@tyrfingr.is>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef __6502_CPU_H
#define __6502_CPU_H

#include "ram.h"


#define DEBUG	1


const uint8_t	FLAG_CARRY = 1 << 0;
const uint8_t	FLAG_ZERO = 1 << 1;
const uint8_t	FLAG_INT_DISABLE = 1 << 2;
const uint8_t	FLAG_DECIMAL = 1 << 3;
const uint8_t	FLAG_BREAK = 1 << 4;
const uint8_t	FLAG_EXPANSION = 1 << 5;
const uint8_t	FLAG_OVERFLOW = 1 << 6;
const uint8_t	FLAG_NEGATIVE = 1 << 7;


typedef uint8_t		cpu_register8;
typedef uint16_t	cpu_register16;


class CPU {
	public:
		CPU();
		CPU(size_t);

		void dump_registers(void);
		void dump_memory(void);
		void run(bool);
		bool step(void);
		void start_pc(uint16_t);

		// Memory access; use this to load a memory image or
		// write a memory image out.
		void load(const void *, uint16_t, uint16_t);
		void store(void *, uint16_t, uint16_t);
		uint8_t DMA(uint16_t);
		void DMA(uint16_t, uint8_t);
	private:
		cpu_register8	a;
		cpu_register8	x;
		cpu_register8	y;
		cpu_register8	p;
		cpu_register8	s;
		cpu_register16	pc;
		RAM		ram;
		size_t		steps;

		// CPU control
		void		reset_registers(void);
		void		instrc01(uint8_t);
		void		instrc10(uint8_t);
		void		instrc00(uint8_t);
		uint8_t		read_immed();
		uint16_t	read_addr0(uint8_t); // cc = 00
		uint16_t	read_addr1(uint8_t); // cc = 01
		uint16_t	read_addr2(uint8_t); // cc = 10

		// PC instructions
		void step_pc(void);
		void step_pc(uint8_t);

		// status register
		void BRK(void);
		void CLC(void);
		void CLD(void);
		void CLI(void);
		void CLV(void);
		void SEC(void);
		void SED(void);
		void SEI(void);

		// Instructions
		void ADCI(uint8_t);
		void AND(uint8_t);
		void CMP(uint8_t);
		void CPX(uint8_t);
		void CPY(uint8_t);
		void DEX(void);
		void EOR(uint8_t);
		void INX(void);
		void INY(void);
		void LDA(uint8_t);
		void LDX(uint8_t);
		void LDY(uint8_t);
		void ORA(uint8_t);
		void STA(uint8_t);
		void STX(uint8_t);
		void STY(uint8_t);
		void TAX(void);
		void TXA(void);

		// Branching / jumping
		void BPL(uint8_t);
		void BMI(uint8_t);
		void BVC(uint8_t);
		void BVS(uint8_t);
		void BCC(uint8_t);
		void BCS(uint8_t);
		void BNE(uint8_t);
		void BEQ(uint8_t);
		void JMP(void);
		void JSR(void);
		void RTS(void);

		// Stack
		void PHA(void);
		void PLA(void);

};


#endif
