/* theory of operation
 *
 * at the start, there's a single process. core_init() is called system
 * initialisation. it starts a thread, the switcher thread.
 *
 * the main thread runs the "current" aros task. thats all. if left unchecked,
 * it would run the same task forever. thats not much fun though, which is
 * what the switcher thread is for.
 *
 * the switcher thread receives interrupts from a variety of sources (virtual
 * hardware) including a timer thread (for normal task switch operation). when
 * an interrupt occurs, the following happens (high level)
 *
 * - switcher signals main that its time for a context switch
 * - main responds by storing the current context and then waiting
 * - switcher saves the current context into the current aros task
 * - switcher runs the scheduler with the current interrupt state
 * - switcher loads the context for the scheduled task as the new current context
 * - switcher signals main
 * - main wakes up and jumps to the current context
 * - switcher loops and waits for the next interrupt
 */

#define DEBUG 1

#include <aros/system.h>
#include <exec/types.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include <exec/lists.h>
#include <exec/execbase.h>

#include "etask.h"

#include "kernel_intern.h"
#include "syscall.h"
#include "host_debug.h"

#define INT_TIMER   (1<<0)
#define INT_SYSCALL (1<<1)

struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

int irq_enabled;
int in_supervisor;
int sleep_state;

void core_intr_disable(void) {
    D(printf("[kernel] disabling interrupts\n"));
    irq_enabled = 0;
}

void core_intr_enable(void) {
    D(printf("[kernel] enabling interrupts\n"));
    irq_enabled = 1;
}

int core_is_super(void) {
    return in_supervisor;
}

static pthread_t main_thread;
static pthread_t switcher_thread;
static pthread_t timer_thread;

static unsigned long timer_period;

static pthread_mutex_t irq_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t irq_cond = PTHREAD_COND_INITIALIZER;
static uint32_t irq_bits;

static sem_t main_sem;
static sem_t switcher_sem;

static syscall_id_t syscall;

void core_syscall(syscall_id_t type) {
    syscall = type;

    pthread_mutex_lock(&irq_lock);
    irq_bits |= INT_SYSCALL;
    pthread_mutex_unlock(&irq_lock);

    pthread_cond_signal(&irq_cond);
}

static void *timer_entry(void *arg) {
    struct timespec ts;

    while (1) {
        D(printf("[kernel:timer] sleeping\n"));

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 1000000000 / timer_period;
        if (ts.tv_nsec > 999999999) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);

        D(printf("[kernel:timer] timer expiry, triggering timer interrupt\n"));

        pthread_mutex_lock(&irq_lock);
        irq_bits |= INT_TIMER;
        pthread_mutex_unlock(&irq_lock);

        pthread_cond_signal(&irq_cond);
    }
}

static void *switcher_entry(void *arg) {
    uint32_t irq_bits_current;

    while (1) {
        D(printf("[kernel:switcher] waiting for interrupts\n"));

        /* wait for an interrupt */
        pthread_mutex_lock(&irq_lock);
        while (irq_bits == 0) pthread_cond_wait(&irq_cond, &irq_lock);

        /* save the current interrupt bits */
        irq_bits_current = irq_bits;
        irq_bits = 0;

        /* if interrupts are disabled and this wasn't a syscall request then
         * we don't want to do anything here */
        if (!irq_enabled && !(irq_bits_current & INT_SYSCALL)) {
            pthread_mutex_unlock(&irq_lock);
            D(printf("[kernel:switcher] interrupts disabled, looping\n"));
            continue;
        }

        /* XXX if we ever have a syscall that would not require a context
         * switch, then don't bother stopping and restarting the main task */

        /* tell the main task to stop and wait for its signal to proceed */
        if (sleep_state != ss_SLEEPING) {
            sem_post(&main_sem);
            pthread_kill(main_thread, SIGUSR1);
            sem_wait(&switcher_sem);
        }

        /* allow further interrupts to arrive */
        pthread_mutex_unlock(&irq_lock);

        D(printf("[kernel:switcher] interrupt received, irq bits are 0x%x\n", irq_bits_current));

        in_supervisor++;

        if (irq_bits_current & INT_SYSCALL) {
            switch (syscall) {
                case sc_CAUSE:
                    core_Cause(*SysBasePtr);
                    break;

                case sc_DISPATCH:
                    core_Dispatch();
                    break;

                case sc_SWITCH:
                    core_Switch();
                    break;

                case sc_SCHEDULE:
                    core_Schedule();
                    break;
            }
        }

        /* if interrupts are enabled, then its time to schedule a new task */
        if (irq_enabled)
            core_ExitInterrupt();

        in_supervisor--;

        /* if we're sleeping then we don't want to wake the main task just now */
        if (sleep_state != ss_RUNNING)
            sleep_state = ss_SLEEPING;

        /* ready to go, give the main task a kick */
        else
            sem_post(&main_sem);
    }

    return NULL;
}

static void main_switch_handler(int signo, siginfo_t *si, void *vctx) {
    /* make sure we were signalled by the switcher thread and not somewhere else */
    if (sem_trywait(&main_sem) < 0)
        return;

    /* switcher thread is now waiting for us. save the context into the task struct */
    getcontext(GetIntETask((*SysBasePtr)->ThisTask)->iet_Context);

    /* tell the switcher to proceed */
    sem_post(&switcher_sem);

    /* wait for it to run the scheduler and whatever else */
    sem_wait(&main_sem);

    /* new task has been switched in, jump to it using its context */
    setcontext(GetIntETask((*SysBasePtr)->ThisTask)->iet_Context);
}

int core_init(unsigned long TimerPeriod, struct ExecBase **SysBasePointer, struct KernelBase **KernelBasePointer) {
    struct sigaction sa;
    pthread_attr_t thread_attrs;

    D(printf("[kernel] initialising interrupts and task switching\n"));

    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;

    irq_enabled = 0;
    in_supervisor = 0;
    sleep_state = 0;

    timer_period = TimerPeriod;

    irq_bits = 0;

    sem_init(&main_sem, 0, 0);
    sem_init(&switcher_sem, 0, 0);

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = main_switch_handler;
    sigaction(SIGUSR1, &sa, NULL);

    main_thread = pthread_self();

    pthread_attr_init(&thread_attrs);
    pthread_attr_setdetachstate(&thread_attrs, PTHREAD_CREATE_DETACHED);
    pthread_create(&switcher_thread, &thread_attrs, switcher_entry, NULL);
    pthread_create(&timer_thread, &thread_attrs, timer_entry, NULL);

    D(printf("[kernel] threads started, switcher id 0x%x, timer id 0x%x\n", switcher_thread, timer_thread));

    return 0;
}
