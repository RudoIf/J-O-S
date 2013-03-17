
#include <inc/string.h>
#include <inc/lib.h>


void
cputchar(int ch)
{
	char c = ch;

	// Unlike standard Unix's putchar,
	// the cputchar function _always_ outputs to the system console.
	sys_cputs(&c, 1);
}

int
getchar(void)
{
	unsigned char c;
	int r;

	// JOS does, however, support standard _input_ redirection,
	// allowing the user to redirect script files to the shell and such.
	// getchar() reads a character from file descriptor 0.
	r = read(0, &c, 1);
	if (r < 0)
		return r;
	if (r < 1)
		return -E_EOF;
	return c;
}


// "Real" console file descriptor implementation.
// The putchar/getchar functions above will still come here by default,
// but now can be redirected to files, pipes, etc., via the fd layer.

static ssize_t devcons_read(struct Fd*, void*, size_t);
static ssize_t devcons_write(struct Fd*, const void*, size_t);
static int devcons_close(struct Fd*);
static int devcons_stat(struct Fd*, struct Stat*,char rwx);

struct Dev devcons =
{
	.dev_id =	'c',
	.dev_name =	"cons",
	.dev_read =	devcons_read,
	.dev_write =	devcons_write,
	.dev_close =	devcons_close,
	.dev_stat =	devcons_stat
};

int
iscons(int fdnum)
{
	int r;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0)
		return r;
	return fd->fd_dev_id == devcons.dev_id;
}

int
opencons(int name)
{
	int r;
	int fd;
	char*  devname = (char*)malloc(16);
/*
	if ((r = fd_alloc(&fd)) < 0)
		return r;
	if ((r = sys_page_alloc(0, fd, PTE_P|PTE_U|PTE_W|PTE_SHARE)) < 0)
		return r;
	fd->fd_dev_id = devcons.dev_id;
	fd->fd_omode = O_RDWR;
	*/

	devname[0] = '\0';
	if(name == 0)
		strcpy(devname,"/dev/console");
	else{
		char ttyname[9];
		strcpy(ttyname,"/dev/tty");
		sprintf(devname,"%s%d",ttyname,name);
	}
	cprintf("devname:%s\n",devname);
	if ((fd = open(devname, O_RDWR|O_MKDEV|O_TRUNC)) < 0) {
		cprintf("create dev %s for rw: %e", "/console", fd);
		exit();
	}

	free(devname);
	if (fd != 0) {
		dup(fd, 0);
		close(fd);
	}

	return fd;
}

static ssize_t
devcons_read(struct Fd *fd, void *vbuf, size_t n)
{
	int c;

	if (n == 0)
		return 0;

	while ((c = sys_cgetc()) == 0)
		sys_yield();
	if (c < 0)
		return c;
	if (c == 0x04)	// ctl-d is eof
		return 0;
	*(char*)vbuf = c;
	return 1;
}

static ssize_t
devcons_write(struct Fd *fd, const void *vbuf, size_t n)
{
	int tot, m;
	char buf[128];

	// mistake: have to nul-terminate arg to sys_cputs,
	// so we have to copy vbuf into buf in chunks and nul-terminate.
	for (tot = 0; tot < n; tot += m) {
		m = n - tot;
		if (m > sizeof(buf) - 1)
			m = sizeof(buf) - 1;
		memmove(buf, (char*)vbuf + tot, m);
		sys_cputs(buf, m);
	}
	return tot;
}

static int
devcons_close(struct Fd *fd)
{
	USED(fd);

	return 0;
}

static int
devcons_stat(struct Fd *fd, struct Stat *stat,char rwx)
{
	int r = 0;
	r = devfile.dev_stat(fd,stat,rwx); 	
	stat->st_isdir = FTYPE_DEV;
	strcpy(stat->st_name, "<cons>");
	return r;
}

