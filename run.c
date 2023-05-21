#include "commands.c"

void run() {
	load_data();
	Command cmd;
	while(1) {
		cmd = parse_cmd(read_cmd());
		cmd.do_command();
		reg_dump();
		printf("\n\n\n");
	}
}

int main() {
	w_write(ostat, 0177777);
	log_level = TRACE;
	run();
	return 0;
}
