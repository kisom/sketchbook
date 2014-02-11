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

#include "Arduino.h"
#include "cpu.h"
#include "ram.h"
#include <cp437.h>


// The aaa bits of instructions.
static const uint8_t	aaa = (1 << 7) | (1 << 6) | (1 << 5);

// The bbb bits of instructions.
static const uint8_t	bbb = (1 << 4) | (1 << 3) | (1 << 2);

// The cc bits of instructions.
static const uint8_t	cc = (1 << 1) | (1 << 0);

// C01 address modes
static const uint8_t	C01_MODE_IIZPX = 0;
static const uint8_t	C01_MODE_ZP = 1;
static const uint8_t	C01_MODE_IMM = 2;
static const uint8_t	C01_MODE_ABS = 3;
static const uint8_t	C01_MODE_IIZPY = 4;
static const uint8_t	C01_MODE_ZPX = 5;
static const uint8_t	C01_MODE_ABSY = 6;
static const uint8_t	C01_MODE_ABSX = 7;

// C10 address modes
static const uint8_t	C10_MODE_IMM = 0;
static const uint8_t	C10_MODE_ZP = 1;
static const uint8_t	C10_MODE_ACC = 2;
static const uint8_t	C10_MODE_ABS = 3;
static const uint8_t	C10_MODE_ZPX = 5;
static const uint8_t	C10_MODE_ZPY = 8;
static const uint8_t	C10_MODE_ABSX = 7;

static int screen_x = 0;
static int screen_y = 0;


#define SCREEN_Y_LIMIT  20

static void clearscr(void) {
  static char line[51];
  memset(line, 0x20, 50);
  for (int i = 0; i <= SCREEN_Y_LIMIT; i++) {
    drawstr(screen_addr(0, i), line); 
  }
}

static int nextaddr() {
  int addr = 0;
  static int y = 0;

  if (y >= 20) {
    clearscr();
    y = 0;
  }  
  addr = screen_addr(0, y);
  y++;
  return addr; 
}

static void
print(const char *s)
{
  int addr = nextaddr();
  drawstr(addr, s);
  delay(10);
  Serial.println(s);
}


static char *
status_flags(cpu_register8 p)
{
  char *status = new char[9];
  memset(status, 0x30, 8);
  status[8] = 0;

  if (p & FLAG_NEGATIVE)
    status[0] = '1';
  if (p & FLAG_OVERFLOW)
    status[1] = '1';
  if (p & FLAG_EXPANSION)
    status[2] = '1';
  if (p & FLAG_BREAK)
    status[3] = '1';
  if (p & FLAG_DECIMAL)
    status[4] = '1';
  if (p & FLAG_INT_DISABLE)
    status[5] = '1';
  if (p & FLAG_ZERO)
    status[6] = '1';
  if (p & FLAG_CARRY)
    status[7] = '1';
  return status;
}


static uint8_t
overflow(uint8_t a, uint8_t b)
{
  return !((a ^ b) & 0x80);
}


CPU::CPU(size_t memory)
{
  print("INIT MEMORY\0");
  this->ram = RAM(memory);
  this->reset_registers();
  //ram.reset();
}


CPU::CPU()
{
  print("INIT MEMORY");
  this->reset_registers();
  this->ram = RAM();
  //ram.reset();
}


void
CPU::reset_registers()
{
  print("RESET REGISTERS\0");
  this->a = 0;
  this->x = 0;
  this->y = 0;
  this->p = FLAG_EXPANSION;
  this->s = 0xff;
  this->pc = 0;
}


static void
dump_register(const char *name, unsigned int value)
{
  Serial.print("\t\0");
  Serial.print(name);
  Serial.print(": \0");
  Serial.println(value, HEX);
}

void
CPU::dump_registers()
{
  size_t	 size = this->ram.size();
  char	*status = status_flags(this->p);

  Serial.println("\r\nREGISTER DUMP\r\n");
  Serial.print("RAM: \0");
  Serial.print(size);
  Serial.println(" bytes");

  dump_register("  A", (unsigned int)this->a);
  dump_register("  X", (unsigned int)this->x);
  dump_register("  Y", (unsigned int)this->y);
  dump_register("  P", (unsigned int)this->p);
  Serial.println("\tFLA: NV-BIDZC");
  Serial.print("\t     \0");
  Serial.println(status);
  dump_register("  S", (unsigned int)this->s);
  dump_register(" PC", (unsigned int)this->pc);

  delete status;
}


/*
void
 CPU::dump_memory()
 {
 	this->ram.dump();
 }
 */

void
CPU::run(bool trace)
{
  while(this->step()) {
    if (trace) {
      this->dump_registers();
    }
  }
}

void
CPU::load(const void *src, uint16_t offset, uint16_t len)
{
  this->ram.load(src, offset, len);
}


void
CPU::store(void *dest, uint16_t offset, uint16_t len)
{
  this->ram.store(dest, offset, len);
}


void
CPU::step_pc()
{
  this->pc++;
}


void
CPU::step_pc(uint8_t n)
{
  if (n & 0x80)
    this->pc -= uint8_t(~n) + 1;
  else
    this->pc += n;
}


void
CPU::start_pc(uint16_t loc)
{
  this->pc = loc;
}


// ADC was taken by the Arduino library, so it's renamed ADCI here.
void
CPU::ADCI(uint8_t op)
{
	uint8_t	v = 0;
	print("OP: ADC\0");

	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		print("MODE: IMM\0");
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
		break;
	}

	if ((uint8_t)(this->a + v) < (this->a))
		this->p |= FLAG_CARRY;
	if (overflow(this->a, v))
		this->p |= FLAG_OVERFLOW;

	this->a += v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;

	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;

	if (!(this->a & 0x80))
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::AND(uint8_t op)
{
	uint8_t	v;
	print("OP: AND\0");

	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		v = this->read_immed();
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}
	this->a &= v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
}


void
CPU::CMP(uint8_t op)
{
	uint8_t	v = 0;
	print("OP: CMP\0");

	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);
	if (this->a < v) {
		if ((this->a - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->a == v) {
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->a > v) {
		this->p |= FLAG_CARRY;
		if ((this->a - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::CPX(uint8_t op)
{
	uint8_t		v;

	print("OP: CPX\0");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);

	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}

	if (this->x < v) {
		if ((this->x - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->x == v) {
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->x > v) {
		this->p |= FLAG_CARRY;
		if ((this->x - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::CPY(uint8_t op)
{
	uint8_t		v;

	print("OP: CPY\0");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);

	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr0((op & bbb) >> 2));
	}

	if (this->y < v) {
		if ((this->y - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->y == v) {
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->y > v) {
		this->p |= FLAG_CARRY;
		if ((this->y - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::DEX()
{
	print("OP: DEX\0");
	this->x--;
	if (this->x == 0)
		this->p |= FLAG_ZERO;
	else if (this->x == 0xFF)
		this->p |= FLAG_CARRY;
}


void
CPU::EOR(uint8_t op)
{
	print("OP: EOR\0");
	uint8_t	v;

	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		v = read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}

	this->a ^= v;
	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
}


void
CPU::INX()
{
	print("OP: INX\0");
	this->x++;
	if (this->x == 0)
		this->p |= (FLAG_ZERO | FLAG_CARRY);
}


void
CPU::INY()
{
	print("OP: INY\0");
	this->y++;
	if (this->y == 0)
		this->p |= (FLAG_ZERO | FLAG_CARRY);
}


void
CPU::LDA(uint8_t op)
{
	print("OP: LDA\0");
	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		print("MODE: IMM\0");
		this->a = this->read_immed();
		break;
	default:
		this->a = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}
	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;

}


void
CPU::LDX(uint8_t op)
{
	print("OP: LDX\0");
	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		print("MODE: IMM\0");
		this->x = this->read_immed();
		break;
	default:
		print("INVALID ADDRESSING MODE\0");
	}
	if (this->x == 0)
		this->p |= FLAG_ZERO;
	if (this->x & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::LDY(uint8_t op)
{
	print("OP: LDY\0");
	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		print("MODE: IMM\0");
		this->y = this->read_immed();
		break;
	default:
		print("INVALID ADDRESSING MODE\0");
	}
	if (this->y == 0)
		this->p |= FLAG_ZERO;
	if (this->y & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::ORA(uint8_t op)
{
	uint8_t	v;
	print("OP: ORA\0");

	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		v = this->read_immed();
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}
	this->a |= v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
}


void
CPU::STA(uint8_t op)
{
	print("OP: STA\0");
	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		print("MODE: IMM\0");
		this->ram.poke((uint16_t)this->read_immed() & 0xff, this->a);
		break;
	default:
		this->ram.poke(this->read_addr1((op & bbb) >> 2), this->a);
		return;
	}
}


void
CPU::STX(uint8_t op)
{
	print("OP: STX\0");
	switch ((op & bbb) >> 2) {
	default:
		this->ram.poke(this->read_addr2((op & bbb) >> 2), this->x);
	}
}


void
CPU::STY(uint8_t op)
{
	print("OP: STY\0");
	switch ((op & bbb) >> 2) {
	default:
		this->ram.poke(this->read_addr0((op & bbb) >> 2), this->y);
	}
}


void
CPU::TAX()
{
	print("OP: TAX\0");
	this->x = this->a;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
	if (this->a == 0)
		this->p |= FLAG_ZERO;
}


void
CPU::TXA()
{
	print("OP: TXA\0");
	this->a = this->x;
	if (this->x & 0x80)
		this->p |= FLAG_NEGATIVE;
	if (this->x == 0)
		this->p |= FLAG_ZERO;
}


/*
 * CPU flag set/clear methods.
 */

void
CPU::BRK()
{
	print("BRK\0");
	this->p |= FLAG_BREAK;
}


void
CPU::CLC()
{
	print("CLC\0");
	this->p &= ~FLAG_CARRY;
}


void
CPU::SEC()
{
	print("SEC\0");
	this->p |= FLAG_CARRY;
}


void
CPU::CLD()
{
	print("CLD\0");
	this->p &= ~FLAG_DECIMAL;
}


void
CPU::SED()
{
	print("SED\0");
	this->p |= FLAG_DECIMAL;
}


void
CPU::CLI()
{
	print("CLI\0");
	this->p &= ~FLAG_INT_DISABLE;
}


void
CPU::SEI()
{
	print("SEI\0");
	this->p |= FLAG_INT_DISABLE;
}


void
CPU::CLV()
{
	print("CLV\0");
	this->p &= ~FLAG_OVERFLOW;
}


/*
 * branching instructions
 */


void
CPU::BPL(uint8_t n)
{
	print("OP: BPL\0");
	if (this->p & FLAG_NEGATIVE)
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BMI(uint8_t n)
{
	print("OP: BMI\0");
	if (!(this->p & FLAG_NEGATIVE))
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BVC(uint8_t n)
{
	print("OP: BVC\0");
	if (this->p & FLAG_OVERFLOW)
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BVS(uint8_t n)
{
	print("OP: BVS\0");
	if (!(this->p & FLAG_OVERFLOW))
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BCC(uint8_t n)
{
	print("OP: BCC\0");
	if (this->p & FLAG_CARRY)
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BCS(uint8_t n)
{
	print("OP: BCS\0");
	if (!(this->p & FLAG_CARRY))
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BNE(uint8_t n)
{
	print("OP: BNE\0");
	if (this->p & FLAG_ZERO)
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::BEQ(uint8_t n)
{
	print("OP: BEQ\0");
	if (!(this->p & FLAG_ZERO))
		return;
	print("BRANCH\0");
	this->step_pc(n);
}


void
CPU::JMP()
{
	print("OP: JMP\0");
	uint16_t	addr = this->read_addr1(C01_MODE_ABS);
	this->pc = addr;
}


static uint16_t
stack_addr(uint8_t sp)
{
	return (1 << 8) + sp;
}


void
CPU::JSR()
{
	print("OP: JSR\0");
	uint16_t	jaddr = this->read_addr1(C01_MODE_ABS);
	uint16_t	addr = this->pc-1;

	this->ram.poke(stack_addr(this->s--), (uint8_t)(addr >> 8));
	this->ram.poke(stack_addr(this->s--), (uint8_t)(addr << 8 >> 8));
	this->pc = jaddr;
}


void
CPU::RTS()
{
	print("OP: RTS\0");
	uint16_t	addr;
	addr = this->ram.peek(stack_addr(++this->s));
	addr += (this->ram.peek(stack_addr(++this->s)) << 8);
	this->pc = addr+1;
}


/*
 * Stack instructions.
 */


void
CPU::PHA()
{
	print("OP: PHA\0");
	this->ram.poke((0x01 << 8) + this->s, this->a);
	this->s--;
}


void
CPU::PLA()
{
	print("OP: PLA\0");
	this->s++;
	this->a = this->ram.peek((0x01 << 8) + this->s);
}


/*
 * Instruction processing (reading, parsing, and handling opcodes).
 */


bool
CPU::step()
{
	uint8_t		op;

        Serial.print("PEEK 0x00FE: ");
        Serial.println(this->ram.peek(0x00fe), HEX);
	op = this->ram.peek(this->pc);
	this->step_pc();
	this->steps++;

	// Scan single-byte opcodes first
	switch (op) {
	case 0x00: // BRK
		this->BRK();
		return false;
	case 0x10: // BPL
		this->BPL(this->read_immed());
		return true;
	case 0x18: // CLC
		this->CLC();
		return true;
	case 0x20: // JSR
		this->JSR();
		return true;
	case 0x30: // BMI
		this->BMI(this->read_immed());
		return true;
	case 0x48: // PHA
		this->PHA();
		return true;
	case 0x4C: // JMP
		this->JMP();
		return true;
	case 0x50: // BVC
		this->BVC(this->read_immed());
		return true;
	case 0x60: // RTS
		this->RTS();
		return true;
	case 0x68: // PLA
		this->PLA();
		return true;
	case 0x70: // BVS
		this->BVS(this->read_immed());
		return true;
	case 0x8a: // TXA
		this->TXA();
		return true;
	case 0x90: // BCC
		this->BCC(this->read_immed());
		return true;
	case 0xB0: // BCC
		this->BCS(this->read_immed());
		return true;
	case 0xC8: // INY
		this->INY();
		return true;
	case 0xCA: // DEX
		this->DEX();
		return true;
	case 0xD0: // BNE
		this->BNE(this->read_immed());
		return true;
	case 0xE8: // INX
		this->INX();
		return true;
	}

	switch (op & cc) {
	case 0x00:
		this->instrc00(op);
		return true;
	case 0x01:
		this->instrc01(op);
		return true;
	case 0x02:
		this->instrc10(op);
		return true;
	default:
		break;
	}
	return false;
}


void
CPU::instrc01(uint8_t op)
{
	switch (op >> 5) {
	case 0x0: // ORA
		this->ORA(op);
		break;
	case 0x1: // AND
		this->AND(op);
		break;
	case 0x3: // ADC
		this->ADCI(op);
		return;
	case 0x4: // STA
		this->STA(op);
		return;
	case 0x5: // LDA
		this->LDA(op);
		return;
	case 0x6: // CMP
		this->CMP(op);
		return;
	default:
		break;
	}
}


void
CPU::instrc10(uint8_t op)
{
	switch (op >> 5) {
	case 0x04:
		this->STX(op);
		break;
	case 0x05:	// LDX / TAX
		if (((op & bbb) >> 2) == C10_MODE_ACC)
			this->TAX();
		else
			this->LDX(op);
		break;
	case 0x06:
		break;
	default:
		break;
	}
}


void
CPU::instrc00(uint8_t op)
{
	switch (op >> 5) {
	case 0x04:
		this->STY(op);
		break;
	case 0x05:
		this->LDY(op);
		break;
	case 0x06:
		this->CPY(op);
		break;
	case 0x07:
		this->CPX(op);
		break;
	default:
		this->dump_registers();
	}
}


uint8_t
CPU::read_immed()
{
	uint8_t	v;
	v = this->ram.peek(this->pc);
	this->step_pc();
	return v;
}


uint16_t
CPU::read_addr1(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C01_MODE_IIZPX:
                print("MODE: IIZPX\0");
		addr = this->read_immed();
		addr += this->x;
		addr = this->ram.peek(addr) + (this->ram.peek(addr+1)<<8);
		break;
	case C01_MODE_ZP:
                print("MODE: ZP\0");
		addr = this->read_immed();
		break;
	case C01_MODE_ABS:
		print("MODE: ABS\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	case C01_MODE_IIZPY:
		print("MODE: IIZPY\0");
		addr = this->read_immed();
		addr = this->ram.peek(addr) + (this->ram.peek(addr+1)<<8);
		addr += this->y;
		break;
	case C01_MODE_ZPX:
		print("MODE: ZPX\0");
		addr = (uint8_t)(this->read_immed() + this->x);
		break;
	case C01_MODE_ABSY:
		print("MODE: ABSY\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->y;
		break;
	case C01_MODE_ABSX:
		print("MODE: ABSX\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->x;
		break;
	default:
		print("INVALID ADDRESSING MODE\0");
		addr = 0;
	}

	return addr;
}


uint16_t
CPU::read_addr2(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C10_MODE_ZP:
		print("MODE: ZP\0");
		addr = this->read_immed();
		break;
	case C10_MODE_ABS:
		print("MODE: ABS\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	case C10_MODE_ZPX:
		print("MODE: ZPX\0");
		addr = (uint8_t)(this->read_immed() + this->x);
		break;
	case C10_MODE_ZPY:
		print("MODE: ZPY\0");
		addr = (uint8_t)(this->read_immed() + this->y);
		break;
	case C10_MODE_ABSX:
		print("MODE: ABSX\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->x;
		break;
	default:
		print("INVALID ADDRESSING MODE\0");
		addr = 0;
	}

	return addr;
}


uint16_t
CPU::read_addr0(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C10_MODE_ZP:
		print("MODE: ZP\0");
		addr = this->read_immed();
		break;
	case C10_MODE_ABS:
		print("MODE: ABS\0");
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	default:
		print("INVALID ADDRESSING MODE\0");
		addr = 0;
	}

	return addr;
}


// The DMA read function allows the host to peer into the CPU's memory.
uint8_t
CPU::DMA(uint16_t loc)
{
	return this->ram.peek(loc);
}


// The DMA store function allows the host to manipulate the VM's
// memory. This might be useful, i.e. for graphics adapters and input
// devices.
void
CPU::DMA(uint16_t loc, uint8_t val)
{
	this->ram.poke(loc, val);
}
