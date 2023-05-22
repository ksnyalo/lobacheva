#include <string.h>
#include <assert.h>
#include "run.c"


void test_mem();
void test_parse_cmd();
void test_mode0();
void test_mode0_mov();
void test_mode0_add();
void test_mode1_toreg();
void test_mode1_fromreg();
void test_mode1_full();
void test_mode2();
void test_mode2_byte_reg6();
void test_mode3();
void test_mode4();
void test_mode5();
void test_mode6();
void test_sob();
void test_cmp();



// чтение и запись байтов и слов
void test_mem() {
	Adress a;
	byte b0, b1, bres, b1res, b2res;
	word w, wres;


	// пишем байт, читаем байт
	fprintf(stderr, "Пишем и читаем байт по четному адресу\n");
	a = 100;
	b0 = 0x12;
	b_write(a, b0);
	bres = b_read(a);
	// отладочная печать a, b0, bres
	fprintf(stderr, "a=%06o b0=%hhx bres=%hhx\n\n", a, b0, bres);
	assert(b0 == bres);

	fprintf(stderr, "Пишем и читаем байт по нечетному адресу\n");
	a = 103;
	b0 = 0x12;
	b_write(a, b0);
	bres = b_read(a);
	// отладочная печать a, b0, bres
	fprintf(stderr, "a=%06o b0=%hhx bres=%hhx\n\n", a, b0, bres);
	assert(b0 == bres);


	// пишем слово, читаем слово
	fprintf(stderr, "Пишем и читаем слово\n");
	a = 100;        // другой адрес
	w = 0x3456;
	w_write(a, w);
	wres = w_read(a);
	// отладочная печать a, w, wres
	fprintf(stderr, "a=%06o w=%04x wres=%04x\n\n", a, w, wres);
	assert(w == wres);


	// пишем 2 байта, читаем 1 слово
	fprintf(stderr, "Пишем 2 байта, читаем слово\n");
	a = 4;        // другой адрес
	w = 0xa1b2;
	// little-endian, младшие разряды по меньшему адресу
	b0 = 0xb2;
	b1 = 0xa1;
	b_write(a, b0);
	b_write(a+1, b1);
	wres = w_read(a);
	// отладочная печать a, b1, b0, w, wres
	fprintf(stderr, "a=%06o b1=%02hhx b0=%02hhx w=%04x wres=%04x\n\n", a, b1, b0, w, wres);
	assert(w == wres);


	// пишем слово, читаем 2 байта
	fprintf(stderr, "Пишем слово, читаем 2 байта\n");
	a = 10;        // другой адрес
	w = 0xc3b2;
	b0 = 0xb2;
	b1 = 0xc3;
	// little-endian, младшие разряды по меньшему адресу
	w_write(a, w);
	b1res = b_read(a);
	b2res = b_read(a+1);
	// отладочная печать a, w, b1res, b2res
	fprintf(stderr, "a=%06o b2res=%02hhx b1res=%02hhx w=%04x\n", a, b2res, b1res, w);
	assert(b0 == b1res);
	assert(b1 == b2res);

	// еще тест
	fprintf(stderr, "Пишем 2 байта, читаем слово\n");
	a = 11;        // другой адрес
	w = 0xd3f5;
	// little-endian, младшие разряды по меньшему адресу
	b0 = 0xf5;
	b1 = 0xd3;
	b_write(a, b1);
	b_write(a-1, b0);
	wres = w_read(a-1);
	// отладочная печать
	fprintf(stderr, "a=%06o b1=%02hhx b0=%02hhx wres=%04x\n\n", a, b1, b0, wres);
	assert(w == wres);
}


// правильно читаем команду
void test_parse_cmd() {
	trace(TRACE, "test_parse_cmd\n");
	Command testcmd = parse_cmd(0010403);
	assert(0 == strcmp(testcmd.name, "mov"));
	trace(TRACE, " ...OK\n");
}


// тест на моду 0, разбор аргументов ss и dd в mov R5, R3
void test_mode0() {
	trace(TRACE, "Тест на моду 0, разбор аргументов ss и dd\n");
	reg[3] = 12;  // dd
	reg[5] = 34;  // ss

	Command testcmd = parse_cmd(0010503);
	trace(TRACE, "ss.val:%d, ss.adr:%o, dd.val:%d, dd.adr:%o\n", ss.val, ss.adr, dd.val, dd.adr);

	assert(ss.val == 34);
	assert(ss.adr == 5);
	assert(dd.val == 12);
	assert(dd.adr == 3);

	trace(TRACE, " ...OK\n");
}


// тест, что мода 0 и команды работают верно в mov R5, R3
void test_mode0_mov() {
	trace(TRACE, "Тест на моду 0, mov\n");
	reg[3] = 12;   //dd
	reg[5] = 34;   //ss

	Command testcmd = parse_cmd(0010503);
	testcmd.do_command();
	trace(TRACE, "ss.val:%d, ss.adr:%o, dd.val:%d, dd.adr:%o reg[3]:%d, reg[5]:%d\n", ss.val, ss.adr, dd.val, dd.adr, reg[3], reg[5]);

	assert(reg[3] == 34);
	assert(reg[5] == 34);

	trace(TRACE, " ...OK\n");
}

void test_mode0_add() {
	trace(TRACE, "Тест, что add и мода 0 работают верно в mov R5, R3\n");
	reg[3] = 12;	// dd
	reg[5] = 34;	// ss

	Command testcmd = parse_cmd(0060503);
	testcmd.do_command();
	trace(TRACE, "reg[3]:%d, reg[5]:%d\n", reg[3], reg[5]);

	assert(reg[3] == 46);
	assert(reg[5] == 34);

	trace(TRACE, " ...OK\n");
}

// тесты на моду 1
void test_mode1_toreg() {
	trace(TRACE, "Тест для моды 1 вида mov (R5), R3\n");
	reg[3] = 12;   //dd
	reg[5] = 0200; //ss
	w_write(0200, 34);
	Command testcmd = parse_cmd(011503);

	assert(ss.val == 34);
	assert(ss.adr = 0200);
	assert(dd.val == 12);
	assert(dd.adr == 3);

	testcmd.do_command();

	assert(reg[3] == 34);
	assert(reg[5] == 0200); // проверяем, что значение регистра не изменилось
	trace(TRACE, " ...OK\n");
}

void test_mode1_fromreg() {
	trace(TRACE, "Тест для моды 1 вида mov R5, (R3)\n");
	reg[3] = 0202;	// dd
	reg[5] = 12;    // ss
	w_write(0202, 34);
	Command testcmd = parse_cmd(0010513);

	assert(dd.val == 34);
	assert(dd.adr == 0202);
	assert(ss.val == 12);
	assert(ss.adr == 5);

	testcmd.do_command();

	assert(reg[5] == 12);
	assert(w_read(0202) == 12); // проверяем, корректно ли скопировалось
	assert(reg[3] == 0202);     // проверяем, что значение регистра не изменилось
	trace(TRACE, " ...OK\n");
}

void test_mode1_full() {
	trace(TRACE, "Тест для моды 1 вида mov (R5), (R3)\n");
	reg[3] = 0206;	// dd
	reg[5] = 0204;  // ss
	w_write(0204, 34);
	w_write(0206, 12);

	Command testcmd = parse_cmd(0011513);

	assert(dd.val == 12);
	assert(dd.adr == 0206);
	assert(ss.val == 34);
	assert(ss.adr == 0204);

	testcmd.do_command();

	assert(w_read(0204) == 34);
	assert(w_read(0206) == 34);  // проверяем, корректно ли копирование
	assert(reg[3] == 0206);      // проверяем, что значение регистра не изменилось
	assert(reg[5] == 0204);      // проверяем, что значение регистра не изменилось
	trace(TRACE, " ...OK\n");
}

// тесты для моды 2
void test_mode2() {
	trace(TRACE, "Тест для моды 2 вида mov (R5)+, R3\n");
	reg[3] = 12;	// dd
	reg[5] = 0200;  // ss
	w_write(0200, 34);

	Command testcmd = parse_cmd(0012503);

	assert(ss.val == 34);
	assert(ss.adr == 0200);
	assert(dd.val == 12);
	assert(dd.adr == 3);

	testcmd.do_command();

	assert(reg[3] == 34);
	assert(reg[5] == 0202);   // проверяем, что значение регистра увеличилось на 2
	trace(TRACE, " ...OK\n");
}

void test_mode2_byte_reg6() {
	trace(TRACE, "Тест на работу 2 моды для байтовой оперции с 6 регистром вида movb (R6)+ (R2)+\n");

	reg[6] = 0300;  // ss
	reg[2] = 0304;	// dd
	b_write(0300, 34);
	b_write(0304, 12);

	Command testcmd = parse_cmd(0112622);

	assert(dd.val == 12);
	assert(dd.adr == 0304);
	assert(ss.val == 34);
	assert(ss.adr == 0300);

	testcmd.do_command();

	//printf("reg[2] = %o, reg[6] = %o\n", reg[2], reg[6]);
	assert(b_read(0304) == 34); //проверяем, корректно ли скопировалось
	assert(reg[2] == 0305);     // проверяем, что значение увеличилось на 1
	assert(reg[6] == 0302);     // проверяем, что значение увеличилось на 2
	trace(TRACE, " ...OK\n");
}

// тест для моды 3
void test_mode3() {
	trace(TRACE, "Тест для моды 3 вида mov @(R5)+, @(R3)+\n");
	reg[3] = 0126;	// dd
	reg[5] = 0122;  // ss
	w_write(0122, 0200);
	w_write(0200, 34);
	w_write(0126, 0130);
	w_write(0130, 12);

	Command testcmd = parse_cmd(0013533);

	assert(dd.val == 12);
	assert(dd.adr == 0130);
	assert(ss.val == 34);
	assert(ss.adr == 0200);

	testcmd.do_command();

	assert(w_read(0200) == 34);
	assert(w_read(0130) == 34);  //проверяем, корректно ли копирование
	// проверяем, что значения регистров увеличились на 2
	assert(reg[3] == 0130);
	assert(reg[5] == 0124);
	trace(TRACE, " ...OK\n");
}

// тест для моды 4
void test_mode4() {
	trace(TRACE, "Тест для моды 4 с 6 регистром вида movb -(R6), -(R3)\n");
	reg[3] = 0446;	// dd
	reg[6] = 0450;  // ss
	b_write(0445, 12);
	b_write(0446, 34);

	Command testcmd = parse_cmd(0114643);

	assert(dd.val == 12);
	assert(dd.adr == 0445);
	assert(ss.val == 34);
	assert(ss.adr == 0446);

	testcmd.do_command();

	assert(b_read(0445) == 34); //проверяем, корректно ли копирование
	assert(b_read(0446) == 34);

	// проверяем, что значение регистра 3 уменьшилось на 1, регистра 6 на 2
	assert(reg[3] == 0445);
	assert(reg[6] == 0446);
	trace(TRACE, " ...OK\n");
}

//тест для моды 5
void test_mode5() {
	trace(TRACE, "Тест для моды 5 вида mov @-(R5), @-(R3)\n");
	reg[3] = 0276;	// dd
	reg[5] = 0270;  // ss
	w_write(0266, 0372);
	w_write(0372, 34);
	w_write(0274, 0200);
	w_write(0200, 12);

	Command testcmd = parse_cmd(0015553);

	assert(dd.val == 12);
	assert(dd.adr == 0200);
	assert(ss.val == 34);
	assert(ss.adr == 0372);

	testcmd.do_command();

	assert(w_read(0372) == 34); //проверяем прошло ли успешно копирование
	assert(w_read(0200) == 34);
	// проверяем, что значения регистров уменьшились на 2
	assert(reg[3] == 0274);
	assert(reg[5] == 0266);
	trace(TRACE, " ...OK\n");
}

// тест для моды 6
void test_mode6() {
	trace(TRACE, "Тест для моды 6 вида mov 6(R5), 10(R3)\n");
	reg[5] = 0312;  // ss
	reg[3] = 0322;	// dd
	pc = 01000;
	w_write(01002, 06);  // сдвиг
	w_write(0320, 34);   // кладем в уже сдвинутый адрес
	w_write(01004, 010); // сдвиг
	w_write(0332, 12);   // кладем в уже сдвинутый адрес

	Command testcmd = parse_cmd(0016563);

	assert(dd.val == 12);
	assert(dd.adr == 0332);
	assert(ss.val == 34);
	assert(ss.adr == 0320);

	testcmd.do_command();

	assert(w_read(0332) == 34); //проверяем, корректно ли копирование
	assert(w_read(0320) == 34);
	assert(reg[5] == 0312);
	assert(reg[3] == 0322);
	trace(TRACE, " ...OK\n");
}


void test_br() {
	trace(TRACE, "Тест для функции br\n");
	pc  = 0100;
	Command testcmd = parse_cmd(000402);
	testcmd.do_command();
	printf("nn.val = %o, pc = %o\n", xx, pc);
	assert(pc == 0106);

	trace(TRACE, "Тест для функции br с отрицательным сдвигом\n");
	pc  = 0106;
	testcmd = parse_cmd(000774);
	testcmd.do_command();
	printf("nn.val = %d, pc = %o\n", xx, pc);
	assert(pc == 0100);
	trace(TRACE, " ...OK\n");
}

// тест cmp
void test_cmp() {
	trace(TRACE, "Проверка команды cmp\n");
	reg[0] = 25;
	reg[1] = 25;
	parse_cmd(0020001);
	do_cmp();
	printf("psw = %d\n", psw);
	assert(psw == 2);
	trace(TRACE, " ...OK\n");
}

// тест для флагов N, Z, C
void test_flagN() {
	trace(TRACE, "flag N\n");
	reg[3] = 5;     // dd
	reg[5] = -8;    // ss
	Command testcmd = parse_cmd(0060503);
	testcmd.do_command();
	assert(psw == (1<<2));

	reg[5] = 10;
	testcmd = parse_cmd(0060503);
	testcmd.do_command();
	printf("psw = %d", psw);
	assert((psw>>2) == 0);

	reg[5] = -10;
	testcmd = parse_cmd(0060503);
	testcmd.do_command();
	assert(psw == (1<<2));

	trace(TRACE, " ... OK\n");
}


void test_flagZ()
{
	trace(TRACE, "flag Z\n");
	reg[3] = 6;     // dd
	reg[5] = -6;    // ss
	Command cmd = parse_cmd(060503);
	cmd.do_command();
	assert(psw == (1<<1));

	reg[3] = 6;   
	reg[5] = 7;
	cmd = parse_cmd(010503);
	cmd.do_command();
	assert((psw>>1) == 0);

	reg[3] = 5;   
	reg[5] = 0;
	cmd = parse_cmd(010503);
	cmd.do_command();
	assert(psw == (1<<1));

	trace(TRACE, " ... OK\n");  
}

void test_flagC() {
	trace(TRACE, "flag C\n");
	reg[3] = -1;   
	reg[5] = -1;
	Command cmd = parse_cmd(060503);
	cmd.do_command();
	assert(psw == (1<<2 | 1));
	
	reg[3] = 1;   
	reg[5] = -1;
	cmd = parse_cmd(060503);
	cmd.do_command();
	assert(psw == (1<<1));

	trace(TRACE, " ... OK\n");
}


int main() {
	log_level = TRACE;
	test_br();
	test_cmp();
	test_flagN();
	test_flagZ();
	test_flagC();

	test_mem();
	test_parse_cmd();
	test_mode0();
	test_mode0_mov();
	test_mode0_add();
	test_mode1_toreg();
	test_mode1_fromreg();
	test_mode1_full();
	test_mode2();
	test_mode2_byte_reg6();
	test_mode3();
	test_mode4();
	test_mode5();
	test_mode6();
	mem_clear();

	return 0;
}
