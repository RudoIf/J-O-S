#include <inc/lib.h>
#include <inc/pwd.h>
struct {
	char msg1[5000];
	char msg2[1000];
} data = {
	"this is initialized data",
	"so is this"
};

char bss[6000];

int
sum(const char *s, int n)
{
	int i, tot = 0;
	for (i = 0; i < n; i++)
		tot ^= i * s[i];
	return tot;
}

void
umain(int argc, char **argv)
{
	int i, r, x, want;
	char args[256];

	cprintf("init: running\n");

	want = 0xf989e;
	if ((x = sum((char*)&data, sizeof data)) != want)
		cprintf("init: data is not initialized: got sum %08x wanted %08x\n",
			x, want);
	else
		cprintf("init: data seems okay\n");
	if ((x = sum(bss, sizeof bss)) != 0)
		cprintf("bss is not initialized: wanted sum 0 got %08x\n", x);
	else
		cprintf("init: bss seems okay\n");

	// output in one syscall per line to avoid output interleaving 
	strcat(args, "init: args:");
	for (i = 0; i < argc; i++) {
		strcat(args, " '");
		strcat(args, argv[i]);
		strcat(args, "'");
	}
	cprintf("%s\n", args);

	if ((r = opencons(0)) < 0)
		panic("opencons: %e", r);
	close(r);

	if((r = spawnl("/bin/chmod", "chmod","600","/dev/console", (char*)0)) < 0)
		cprintf("init: spawn getty: %e\n", r);

	//set utmp
	if((r=usercheck()) != 0)
		cprintf("init:usercheck wrong\n");
	
	// start the interactive part.Imitate minix.
	if (1) {
		cprintf("init: starting console\n");
		r = spawnl("/bin/getty", "getty", (char*)0);
		if (r < 0) {
			cprintf("init: spawn getty: %e\n", r);
		}
	}
}
