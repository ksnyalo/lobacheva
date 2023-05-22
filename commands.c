#include <string.h>
#include <stdlib.h>
#include "mem.c"
#include "log.c"


// команды
void do_nothing();
void do_halt();
void do_add();
void do_mov();
void do_movb();
void do_sob();
void do_tst();
void do_cmp();
void do_br();
void do_beq();
void do_bpl();
void do_clr();

// флаги
void set_NZ(word, char);
void set_C(word, word);


char psw;
int shift = 0;
signed char xx;
word re, nn;

word odata = 0177566;   // регистр данных дисплея
word ostat = 0177564;   // регистр состояния дисплея

enum {
	NO_PARAMS,
	HAS_DD,
	HAS_SS,
	HAS_R = 1<<2,
	HAS_NN = 1<<3,
	HAS_XX = 1<<4
} params;



struct Argument {
	word val;               // значение аргумента
	Adress adr;             // адрес аргумента
} ss, dd;



typedef struct {
	word mask;
	word opcode;
	char * name;
	void (*do_command)(void);
	char params;
} Command;



Command cmd[] = {
	{0170000, 0060000, "add", do_add, HAS_SS | HAS_DD},
	{0170000, 0010000, "mov", do_mov, HAS_SS | HAS_DD},
	{0170000, 0110000, "movb", do_movb, HAS_SS | HAS_DD},
	{0177777, 0000000, "halt", do_halt, NO_PARAMS},
	{0177000, 0077000, "sob", do_sob, HAS_NN | HAS_R},
	{0177700, 0000400, "br", do_br, HAS_XX},
	{0177700, 0000700, "br", do_br, HAS_XX},
	{0177700, 0001400, "beq", do_beq, HAS_XX},
	{0177700, 0001700, "beq", do_beq, HAS_XX},
	{0177700, 0100000, "bpl", do_bpl, HAS_XX},
	{0177700, 0100300, "bpl", do_bpl, HAS_XX},
	{0170000, 0020000, "cmp", do_cmp, HAS_SS | HAS_DD},
	{0177700, 0105700, "tst", do_tst, HAS_DD},
	{0177000, 0005000, "clr", do_clr, HAS_DD},
	{0000000, 0000000, "unknown", do_nothing, NO_PARAMS}
};



// команды
void do_nothing() {
}

void do_halt() {
	reg_dump();
//	trace(TRACE, "psw = %o\n\n", psw);
	trace(INFO, "THE END!!!\n");
	exit(0);
}

void do_add() {
	word res = ss.val + dd.val;
	w_write(dd.adr, res);
	set_NZ(res, 15);
	set_C(ss.val, dd.val);
}

void do_mov() {
	word res = ss.val;
	w_write(dd.adr, ss.val);
	set_NZ(res, 15);
}

void do_movb() {
	byte res = (byte)ss.val;
	b_write(dd.adr, res);
	set_NZ(res, 7);
}

void do_sob() {
	if(--reg[re] > 0)
		pc -= 2 * nn;
	else
		reg[re] = 0;
	trace(TRACE, "%o\n", pc);
}

void do_br() {
	pc += 2 * xx;
	trace(TRACE, "%o\n", pc);
}

void do_beq() {               // если равны
	if(psw & (1<<1)) {
		do_br();
	}
}

void do_bpl() {               // если положительное
	if((psw & (1<<2)) == 0) {
		 do_br();
	}
}

void do_cmp() {
	word res = ss.val - dd.val;
	set_NZ(res, 15);
	set_C(ss.val, (-dd.val));
}

void do_tst() {
	psw = 0;
	set_NZ(dd.val, 7);
}

void do_clr() {
	dd.val = 0;
}



// работа с флагами
void set_NZ(word w, char shift) {
	psw = 0;
	if((w>>shift) & 1)
		psw = psw | (1<<2);
	else if (w == 0)
		psw = psw | (1<<1);
//	trace(TRACE, "psw = %o\n", psw);
}

void set_C(word w1, word w2) {
	if((w1>>16) & 1 && (w2>>16) & 1)
		psw = psw | 1;
//	trace(TRACE, "psw = %o\n", psw);
}


int get_r(word w) {
	re = w & 7;
	trace(TRACE, "R%o ", re);
	return re;
}



// сдвиг
void get_xx(word w) {
	signed char shift;
	if((w>>7) & 1) {
		shift = w | 0xff00;            // операция со словом, код команды и аргумент "слипаются", отрицательный сдвиг
	}
	else {
		shift = w & 0377;              // в других случаях все хорошо
	}
	xx = w;
	trace(TRACE, "nn = %d, %o\n", xx, pc+2*xx);
}



// мода и регистр
struct Argument get_mr(word w) {
    struct Argument res;
    int r = w & 7;                     // номер регистра
    int mod = (w >> 3) & 7;            // номер моды
    switch (mod) {
		// мода 0, R1
		case 0:
			res.adr = r;                // адрес - номер регистра
			res.val = reg[r];           // значение - число в регистре
			trace(TRACE, "R%d ", r);
			break;

		// мода 1, (R1)
		case 1:
			res.adr = reg[r];           // в регистре адрес
			if((w>>15) & 1) {
				res.val = b_read(res.adr);  // по адресу - значение, байтовая операция
			}
			else
				res.val = w_read(res.adr);  // по адресу - значение
			trace(TRACE, "(R%d) ", r);
			break;

		// мода 2, (R1)+ или #3
		case 2:
			res.adr = reg[r];           // в регистре адрес

			if(((w>>15) & 1) && r != 6 && r != 7) {
				res.val = b_read(res.adr);
				reg[r]++;               // для байтовой операции
			}
			else {
				res.val = w_read(res.adr);
				reg[r] += 2;            // для слова
			}
			if (r == 7)
				trace(TRACE, "#%o ", res.val);
			else
				trace(TRACE, "(R%d)+ ", r);
			break;

		// мода 3, две переадресации (из регистра в память, в которой хранится адрес)
		case 3:
			res.adr = w_read(reg[r]);
			res.val = w_read(res.adr);
			reg[r] += 2;

			if (r == 7) {
				trace(TRACE, "@#%o ", res.adr);
			}
			else
				trace(TRACE, "@(R%d)+ ", r);
			break;

		// мода 4, автодекрементный режим, сначала минусуется
		case 4:
			if(((w>>15) & 1) && r != 6 && r!= 7)
				reg[r]--;
			else {
				reg[r] -= 2;
			}
			res.adr = reg[r];
			res.val = mem[res.adr];
			if (r == 7)
				trace(TRACE, "#%o ", res.val);
			else
				trace(TRACE, "-(R%d) ", r);
			break;

		// мода 5, косвенно-автодекрементный, всегда -2, тк адрес
		case 5:
			reg[r] -= 2;
			res.adr = w_read(reg[r]);
			res.val = w_read(res.adr);

			if (r == 7)
				trace(TRACE, "@#%o ", res.val);
			else
				trace(TRACE, "@-(R%d) ", r);
			break;

		// мода 6, индексный, содержимое регистра + индекс (сдвиг)
		case 6:
			shift = w_read(pc);
			pc += 2;
			res.adr = reg[r];
			res.adr = (res.adr + shift) & 0xffff;
			res.val = mem[res.adr];

			if (r == 7)
				trace(TRACE, "#%o ", res.adr);
			else
				trace(TRACE, " %d(R%d) ", shift, r);
			break;

		// мода 7, косвенно-индексный, то же самое, но в памяти хранится адрес
		case 7:
			shift = w_read(pc);
			pc += 2;
			res.adr = reg[r];
			res.adr = (res.adr + shift) & 0xffff;
			res.adr = mem[res.adr];             // в памяти адрес
			res.val = mem[res.adr];

			if (r == 7)
				trace(TRACE, "@#%o ", res.val);
			else
			trace(TRACE, " @%d(R%d) ", shift, r);
			break;


		default:
			trace(ERROR, "Mode %d not implemented yet!\n", mod);
			exit(1);
    }
    return res;
}


// читаем команду
word read_cmd() {
	word w = w_read(pc);
	trace(TRACE, "%06o %06o: ", pc, w);
	return w;
}

Command parse_cmd(word w) {
	pc += 2;

	int i = 0;
	for(i = 0; ; i++) {
		if((w & cmd[i].mask) == cmd[i].opcode) {
			trace(TRACE, "%s ", cmd[i].name);
			if(cmd[i].params & HAS_SS)
				ss = get_mr(w>>6);
			if(cmd[i].params & HAS_DD)
				dd = get_mr(w);
			if(cmd[i].params & (HAS_NN)) {
				nn = w & 077;
			}
			if(cmd[i].params & HAS_R)
				re = get_r(w>>6) & 7;
			if(cmd[i].params & HAS_XX)
				get_xx(w);

			printf("\n");

			return cmd[i];
		}
	}
	exit(0);
}
