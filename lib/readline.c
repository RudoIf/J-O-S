#include <inc/stdio.h>
#include <inc/error.h>
#include <inc/string.h>


char *
readline(const char *prompt,char* buf,int buflen)
{
	int i, c, echoing,ifshow;	//ifshow decide if cput.etc. passwd

#if JOS_KERNEL
	if (prompt != NULL)
		cprintf("%s", prompt);
#else
	if (prompt != NULL)
		fprintf(1, "%s", prompt);
#endif
	ifshow = 1;
	if(buflen < 0){
	  buflen = -buflen;
	  ifshow = 0;
	}
	i = 0;
	echoing = iscons(0);
	while (1) {
		c = getchar();
		if (c < 0) {
			if (c != -E_EOF)
				cprintf("read error: %e\n", c);
			return NULL;
		} else if ((c == '\b' || c == '\x7f') && i > 0) {
			if (echoing)
				cputchar('\b');
			i--;
		} else if (c >= ' ' && i < buflen-1) {
			if (echoing && ifshow)
				cputchar(c);
			buf[i++] = c;
		} else if (c == '\n' || c == '\r') {
			if (echoing)
				cputchar('\n');
			buf[i] = 0;
			return buf;
		}
	}
}

