// See LICENSE for license details.

#include "syscall.h"
#include "pk.h"
#include "file.h"
#include "frontend.h"
#include "mmap.h"
#include "boot.h"
#include <string.h>
#include <errno.h>

typedef long (*syscall_t)(long, long, long, long, long, long, long);

#define CLOCK_FREQ 1000000000

void sys_exit(int code)
{
  if (current.cycle0)
  {
    size_t dt = rdtime() - current.time0;
    size_t dc = rdcycle() - current.cycle0;
    size_t di = rdinstret() - current.instret0;

    printk("%ld ticks\n", dt);
    printk("%ld cycles\n", dc);
    printk("%ld instructions\n", di);
    printk("%d.%d%d CPI\n", dc / di, 10ULL * dc / di % 10, (100ULL * dc + di / 2) / di % 10);
  }
  shutdown(code);
}

ssize_t sys_read(int fd, char *buf, size_t n)
{
  ssize_t r = -EBADF;
  file_t *f = file_get(fd);

  if (f)
  {
    r = file_read(f, buf, n);
    file_decref(f);
  }

  return r;
}

ssize_t sys_pread(int fd, char *buf, size_t n, off_t offset)
{
  ssize_t r = -EBADF;
  file_t *f = file_get(fd);

  if (f)
  {
    r = file_pread(f, buf, n, offset);
    file_decref(f);
  }

  return r;
}

ssize_t sys_write(int fd, const char *buf, size_t n)
{
  ssize_t r = -EBADF;
  file_t *f = file_get(fd);

  if (f)
  {
    r = file_write(f, buf, n);
    file_decref(f);
  }

  return r;
}

static int at_kfd(int dirfd)
{
  if (dirfd == AT_FDCWD)
    return AT_FDCWD;
  file_t *dir = file_get(dirfd);
  if (dir == NULL)
    return -1;
  return dir->kfd;
}

int sys_openat(int dirfd, const char *name, int flags, int mode)
{
  int kfd = at_kfd(dirfd);
  if (kfd != -1)
  {
    file_t *file = file_openat(kfd, name, flags, mode);
    if (IS_ERR_VALUE(file))
      return PTR_ERR(file);

    int fd = file_dup(file);
    if (fd < 0)
    {
      file_decref(file);
      return -ENOMEM;
    }

    return fd;
  }
  return -EBADF;
}

int sys_open(const char *name, int flags, int mode)
{
  return sys_openat(AT_FDCWD, name, flags, mode);
}

int sys_close(int fd)
{
  int ret = fd_close(fd);
  if (ret < 0)
    return -EBADF;
  return ret;
}

int sys_fstat(int fd, void *st)
{
  int r = -EBADF;
  file_t *f = file_get(fd);

  if (f)
  {
    r = file_stat(f, st);
    file_decref(f);
  }

  return r;
}

ssize_t sys_lseek(int fd, size_t ptr, int dir)
{
  ssize_t r = -EBADF;
  file_t *f = file_get(fd);

  if (f)
  {
    r = file_lseek(f, ptr, dir);
    file_decref(f);
  }

  return r;
}

long sys_fstatat(int dirfd, const char *name, void *st, int flags)
{
  int kfd = at_kfd(dirfd);
  if (kfd != -1)
  {
    struct frontend_stat buf;
    size_t name_size = strlen(name) + 1;
    long ret = frontend_syscall(SYS_fstatat, kfd, va2pa(name), name_size, va2pa(&buf), flags, 0, 0);
    copy_stat(st, &buf);
    return ret;
  }
  return -EBADF;
}

long sys_lstat(const char *name, void *st)
{
  struct frontend_stat buf;
  size_t name_size = strlen(name) + 1;
  long ret = frontend_syscall(SYS_lstat, va2pa(name), name_size, va2pa(&buf), 0, 0, 0, 0);
  copy_stat(st, &buf);
  return ret;
}

long sys_stat(const char *name, void *st)
{
  return sys_fstatat(AT_FDCWD, name, st, 0);
}

long sys_linkat(int old_dirfd, const char *old_name, int new_dirfd, const char *new_name, int flags)
{
  int old_kfd = at_kfd(old_dirfd);
  int new_kfd = at_kfd(new_dirfd);
  if (old_kfd != -1 && new_kfd != -1)
  {
    size_t old_size = strlen(old_name) + 1;
    size_t new_size = strlen(new_name) + 1;
    return frontend_syscall(SYS_linkat, old_kfd, va2pa(old_name), old_size,
                            new_kfd, va2pa(new_name), new_size,
                            flags);
  }
  return -EBADF;
}

int sys_renameat(int old_fd, const char *old_path, int new_fd, const char *new_path)
{
  int old_kfd = at_kfd(old_fd);
  int new_kfd = at_kfd(new_fd);
  if (old_kfd != -1 && new_kfd != -1)
  {
    size_t old_size = strlen(old_path) + 1;
    size_t new_size = strlen(new_path) + 1;
    return frontend_syscall(SYS_renameat, old_kfd, va2pa(old_path), old_size,
                            new_kfd, va2pa(new_path), new_size, 0);
  }
  return -EBADF;
}

long sys_mkdirat(int dirfd, const char *name, int mode)
{
  int kfd = at_kfd(dirfd);
  if (kfd != -1)
  {
    size_t name_size = strlen(name) + 1;
    return frontend_syscall(SYS_mkdirat, kfd, va2pa(name), name_size, mode, 0, 0, 0);
  }
  return -EBADF;
}

long sys_mkdir(const char *name, int mode)
{
  return sys_mkdirat(AT_FDCWD, name, mode);
}

long sys_getcwd(const char *buf, size_t size)
{
  populate_mapping(buf, size, PROT_WRITE);
  return frontend_syscall(SYS_getcwd, va2pa(buf), size, 0, 0, 0, 0, 0);
}

size_t sys_brk(size_t pos)
{
  return do_brk(pos);
}

int sys_uname(void *buf)
{
  const int sz = 65;
  strcpy(buf + 0 * sz, "Proxy Kernel");
  strcpy(buf + 1 * sz, "");
  strcpy(buf + 2 * sz, "4.15.0");
  strcpy(buf + 3 * sz, "");
  strcpy(buf + 4 * sz, "");
  strcpy(buf + 5 * sz, "");
  return 0;
}

int sys_rt_sigaction(int sig, const void *act, void *oact, size_t sssz)
{
  if (oact)
    memset(oact, 0, sizeof(long) * 3);

  return 0;
}

long sys_time(long *loc)
{
  uintptr_t t = rdcycle() / CLOCK_FREQ;
  if (loc)
    *loc = t;
  return t;
}

int sys_times(long *loc)
{
  uintptr_t t = rdcycle();
  kassert(CLOCK_FREQ % 1000000 == 0);
  loc[0] = t / (CLOCK_FREQ / 1000000);
  loc[1] = 0;
  loc[2] = 0;
  loc[3] = 0;

  return 0;
}

ssize_t sys_writev(int fd, const long *iov, int cnt)
{
  ssize_t ret = 0;
  for (int i = 0; i < cnt; i++)
  {
    ssize_t r = sys_write(fd, (void *)iov[2 * i], iov[2 * i + 1]);
    if (r < 0)
      return r;
    ret += r;
  }
  return ret;
}

static int sys_stub_success()
{
  return 0;
}

static int sys_stub_nosys()
{
  return -ENOSYS;
}

void sys_get_init_memsize()
{
  //  your code here:
  //  声明mem_size
  //  printk
  extern uintptr_t mem_size;
  printk("mem_size = 0x%x", mem_size);
}

long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, unsigned long n)
{
  const static void *syscall_table[] = {
      //  your code here:
      //  add get_init_memsize syscall
      [SYS_exit] = sys_exit,
      [SYS_exit_group] = sys_exit,
      [SYS_read] = sys_read,
      [SYS_pread] = sys_pread,
      [SYS_write] = sys_write,
      [SYS_openat] = sys_openat,
      [SYS_close] = sys_close,
      [SYS_fstat] = sys_fstat,
      [SYS_lseek] = sys_lseek,
      [SYS_renameat] = sys_renameat,
      [SYS_mkdirat] = sys_mkdirat,
      [SYS_getcwd] = sys_getcwd,
      [SYS_brk] = sys_brk,
      [SYS_uname] = sys_uname,
      [SYS_prlimit64] = sys_stub_nosys,
      [SYS_rt_sigaction] = sys_rt_sigaction,
      [SYS_times] = sys_times,
      [SYS_writev] = sys_writev,
      [SYS_readlinkat] = sys_stub_nosys,
      [SYS_rt_sigprocmask] = sys_stub_success,
      [SYS_ioctl] = sys_stub_nosys,
      [SYS_getrusage] = sys_stub_nosys,
      [SYS_getrlimit] = sys_stub_nosys,
      [SYS_setrlimit] = sys_stub_nosys,
      [SYS_set_tid_address] = sys_stub_nosys,
      [SYS_set_robust_list] = sys_stub_nosys,
      [SYS_init_memsize] = sys_get_init_memsize,
  };

  syscall_t f = 0;

  if (n < ARRAY_SIZE(syscall_table))
    f = syscall_table[n];
  if (!f)
    panic("bad syscall #%ld!", n);

  return f(a0, a1, a2, a3, a4, a5, n);
}