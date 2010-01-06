/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/thread>
#include <l4/sys/factory>
#include <l4/sys/debugger.h>
#include <l4/cxx/ipc_server>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/util/util.h>

#include <sys/time.h>

#include "arch.h"

#include <errno.h>
#include <signal.h>
#include <cstdio>
#include "sig.h"

// handlers registers with 'signal'
static struct sigaction sigactions[_NSIG];

static char sigthread_stack[2048]; // big stack is thanks to printf
static L4::Cap<L4::Thread> thcap;

static struct itimerval current_itimerval;
static l4_cpu_time_t alarm_timeout;

// -----------------------------------------------------------------------

class Dispatcher
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  void setup_wait(L4::Ipc_iostream &) {}
  void error(long /*ipc_error*/) {}
  int handle_exception(L4::Ipc_iostream &ios);
};

static l4_addr_t get_handler(int signum)
{
  if (signum >= _NSIG)
    return 0;
  if (   (sigactions[signum].sa_flags & SA_SIGINFO)
      && sigactions[signum].sa_sigaction)
    return (l4_addr_t)sigactions[signum].sa_sigaction;
  else if (sigactions[signum].sa_handler)
    return (l4_addr_t)sigactions[signum].sa_handler;
  return 0;
}


asm(
".text                           \n\t"
".global libc_be_sig_return_trap \n\t"
"libc_be_sig_return_trap:        \n\t"
#if defined(ARCH_x86) || defined(ARCH_amd64)
"                          ud2a  \n\t"
#elif defined(ARCH_arm)
".p2align 2                      \n\t"
"word: .long                    0xe1600070 \n\t" // smc
#elif defined(ARCH_ppc32)
"trap                            \n\t"
#else
#error Unsupported arch!
#endif
".previous                       \n\t"
);

extern char libc_be_sig_return_trap[];

static bool range_ok(l4_addr_t start, unsigned long size)
{
  l4_addr_t offset;
  unsigned flags;
  L4::Cap<L4Re::Dataspace> ds;

  return !L4Re::Env::env()->rm()->find(&start, &size, &offset, &flags, &ds)
         && !(flags & L4Re::Rm::Read_only);
}

static bool setup_sig_frame(l4_exc_regs_t *u, int signum)
{
  // put state + pointer to it on stack
  ucontext_t *ucf = (ucontext_t *)(u->sp - sizeof(*ucf));

  /* Check if memory is access is fine */
  if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
    return false;
  //printf("range %lx %x ok\n", (l4_addr_t)ucf, sizeof(*ucf));
  //L4::Cap<L4Re::Debug_obj> d(L4Re::Env::env()->rm().cap());
  //d->debug(0);

  fill_ucontext_frame(ucf, u);
  u->sp = (l4_umword_t)ucf - sizeof(void *);
  *(l4_umword_t *)u->sp = (l4_umword_t)ucf;

  // siginfo_t pointer, we do not have one right currently
  u->sp -= sizeof(siginfo_t *);
  *(l4_umword_t *)u->sp = 0;

  // both types get the signum as the first argument
  u->sp -= sizeof(l4_umword_t);
  *(l4_umword_t *)u->sp = signum;

  u->sp -= sizeof(l4_umword_t);
  *(unsigned long *)u->sp = (unsigned long)libc_be_sig_return_trap;

  return true;
}

int Dispatcher::handle_exception(L4::Ipc_iostream &ios)
{
  l4_exc_regs_t _u;
  l4_exc_regs_t *u = &_u;
  l4_addr_t handler;

  *u = *l4_utcb_exc();

  show_regs(u);

#ifdef ARCH_arm
  if ((u->err & 0x00f00000) == 0x00500000)
#elif defined(ARCH_ppc32)
  if ((u->err & 3) == 4)
#else
  if (u->trapno == 0xff)
#endif
    {
      printf("SIGALRM\n");

      if (!(handler = get_handler(SIGALRM)))
        {
          printf("No signal handler found\n");
          return -L4_ENOREPLY;
        }

      if (!setup_sig_frame(u, SIGALRM))
        {
          printf("Invalid user memory for sigframe...\n");
          return -L4_ENOREPLY;
        }

      l4_utcb_exc_pc_set(u, handler);
      ios.put(*u); // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  // x86: trap6
  if (l4_utcb_exc_pc(u) == (l4_addr_t)libc_be_sig_return_trap)
    {
      // sig-return
      //printf("Sigreturn\n");

      ucontext_t *ucf = (ucontext_t *)(u->sp + sizeof(l4_umword_t) * 3);
      //printf("sp = %lx  uc=%lx\n", u->sp, u->sp + sizeof(l4_umword_t) * 3);

      if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
        {
          printf("Invalid memory...\n");
          return -L4_ENOREPLY;
        }

      fill_utcb_exc(u, ucf);

      ios.put(*u); // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  if (!(handler = get_handler(SIGSEGV)))
    {
      printf("No signal handler found\n");
      return -L4_ENOREPLY;
    }


  printf("Doing SIGSEGV\n");

  if (!setup_sig_frame(u, SIGSEGV))
    {
      printf("Invalid user memory for sigframe...\n");
      return -L4_ENOREPLY;
    }

  show_regs(u);

  l4_utcb_exc_pc_set(u, handler);
  ios.put(*u); // expensive? how to set amount of words in tag without copy?

  //printf("and back\n");
  return -L4_EOK;
}

int Dispatcher::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  switch (t.label())
    {
    case L4_PROTO_EXCEPTION:
      return handle_exception(ios);
    default:
      return -L4_ENOSYS;
    };
}

namespace {

struct Loop_hooks :
  public L4::Ipc_svr::Compound_reply,
  public L4::Ipc_svr::Default_setup_wait
{
  static l4_timeout_t timeout()
  {
    if (alarm_timeout)
      {
	l4_timeout_t t;
	l4_rcv_timeout(l4_timeout_abs(alarm_timeout, 1), &t);
	alarm_timeout = 0;
	return t;
      }

    if (current_itimerval.it_value.tv_sec == 0
	&& current_itimerval.it_value.tv_usec == 0)
      return L4_IPC_NEVER;
    return l4_timeout(L4_IPC_TIMEOUT_NEVER,
	l4util_micros2l4to(current_itimerval.it_value.tv_sec * 1000000 +
	  current_itimerval.it_value.tv_usec));
  }

  void error(l4_msgtag_t res, L4::Ipc_istream &s)
  {
    long ipc_error = l4_ipc_error(res, s.utcb());

    if (ipc_error == L4_IPC_RETIMEOUT)
      {
	l4_msgtag_t t;

	t = L4Re::Env::env()->main_thread()->ex_regs(~0UL, ~0UL,
	    L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
	if (l4_error(t))
	  printf("ex_regs error\n");



	// reload
	current_itimerval.it_value = current_itimerval.it_interval;

	return;
      }
    printf("(unsupported/strange) loopabort: %lx\n", ipc_error);
  }
};


static void __handler_main()
{
  L4::Server<Dispatcher, Loop_hooks> srv(l4_utcb());
  srv.loop();
}
}

static void libsig_be_init(void) __attribute__((constructor));
static void libsig_be_init()
{
  l4_utcb_t *u = (l4_utcb_t *)L4Re::Env::env()->first_free_utcb();

  //L4Re::Env::env()->first_free_utcb((l4_addr_t)u + L4_UTCB_OFFSET);
  // error: passing ‘const L4Re::Env’ as ‘this’ argument of ‘void
  // L4Re::Env::first_free_utcb(l4_addr_t)’ discards qualifiers
  l4re_global_env->first_free_utcb = (l4_addr_t)u + L4_UTCB_OFFSET;


  L4Re::Util::Auto_cap<L4::Thread>::Cap tc = L4Re::Util::cap_alloc.alloc<L4::Thread>();
  if (!tc.is_valid())
    {
      fprintf(stderr, "libsig: Failed to acquire cap\n");
      return;
    }

  int err = l4_error(L4Re::Env::env()->factory()->create_thread(tc.get()));
  if (err < 0)
    {
      fprintf(stderr, "libsig: Failed create thread: %s(%d)\n",
              l4sys_errtostr(err), err);
      return;
    }

  L4::Thread::Attr a;

  a.bind(u, L4Re::This_task);
  a.pager(L4Re::Env::env()->rm());
  a.exc_handler(L4Re::Env::env()->rm());
  tc->control(a);

  l4_umword_t *sp
    = (l4_umword_t *)((char *)sigthread_stack + sizeof(sigthread_stack));
  //printf("stack top %p\n", sp);

  tc->ex_regs(l4_addr_t(__handler_main), l4_addr_t(sp), 0);
  l4_debugger_set_object_name(tc.cap(), "&-");

  thcap = tc.release();

  libsig_be_add_thread(l4re_env()->main_thread);

  return;
}

void libsig_be_set_dbg_name(const char *n)
{
  char s[15];
  snprintf(s, sizeof(s) - 1, "&%s", n);
  s[sizeof(s) - 1] = 0;
  l4_debugger_set_object_name(thcap.cap(), s);
}

void libsig_be_add_thread(l4_cap_idx_t t)
{
  L4::Cap<L4::Thread> tt(t);
  L4::Thread::Attr a;
  a.exc_handler(thcap);
  if (int e = l4_error(tt->control(a)))
    fprintf(stderr, "libsig: thread-control error: %d\n", e);
  printf("Set exc-handler %lx for %lx\n", thcap.cap(), t);
}


extern "C"
sighandler_t signal(int signum, sighandler_t handler) L4_NOTHROW
{
  //printf("Called: %s(%d, %p)\n", __func__, signum, handler);

  if (signum < _NSIG)
    {
      sighandler_t old = sigactions[signum].sa_handler;
      sigactions[signum].sa_handler = handler;
      return old;
    }

  return SIG_ERR;
}

extern "C"
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact) L4_NOTHROW
{
  printf("Called: %s(%d, %p, %p)\n", __func__, signum, act, oldact);

  if (signum == SIGKILL || signum == SIGSTOP)
    {
      errno = EINVAL;
      return -1;
    }

  if (signum < _NSIG)
    {
      if (oldact)
        *oldact = sigactions[signum];
      if (act)
        sigactions[signum] = *act;
      printf("OK\n");
      return 0;
    }

  errno = EINVAL;
  return -1;
}


extern "C"
unsigned int alarm(unsigned int seconds)
{
  //printf("unimplemented: alarm(%u)\n", seconds);

  alarm_timeout = l4re_kip()->clock + seconds * 1000000;

  return 0;
}

extern "C"
pid_t wait(int *status)
{
  printf("unimplemented: wait(%p)\n", status);
  return -1;
}



int getitimer(__itimer_which_t __which,
              struct itimerval *__value) L4_NOTHROW
{
  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  *__value = current_itimerval;

  return 0;
}

int setitimer(__itimer_which_t __which,
              __const struct itimerval *__restrict __new,
              struct itimerval *__restrict __old) L4_NOTHROW
{
  printf("called %s(..)\n", __func__);

  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  if (__old)
    *__old = current_itimerval;

  if (__new->it_value.tv_usec < 0
      || __new->it_value.tv_usec > 999999
      || __new->it_interval.tv_usec < 0
      || __new->it_interval.tv_usec > 999999)
    {
      errno = EINVAL;
      return -1;
    }

  printf("%s: setting stuff\n", __func__);
  current_itimerval = *__new;

  return 0;
}
