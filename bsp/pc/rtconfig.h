/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* RT_NAME_MAX*/
#define RT_NAME_MAX	8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX*/
#define RT_THREAD_PRIORITY_MAX	256

/* Tick per Second*/
#define RT_TICK_PER_SECOND	100

/* SECTION: RT_DEBUG */
/* Thread Debug*/
/* #define RT_THREAD_DEBUG */

/* Using Hook*/
/* #define RT_USING_HOOK */

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex*/
/* #define RT_USING_MUTEX */

/* Using Event*/
/* #define RT_USING_EVENT */

/* Using MailBox*/
#define RT_USING_MAILBOX

/* Using Message Queue*/
/* #define RT_USING_MESSAGEQUEUE */

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
/* #define RT_USING_MEMPOOL */

/* Using Dynamic Heap Management*/
/* #define RT_USING_HEAP */

/* Using Small MM*/
#define RT_USING_SMALL_MEM
#define RT_USING_TINY_SIZE

/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE

/* buffer size for UART reception */
#define RT_UART_RX_BUFFER_SIZE	64

/* Using UART */
#define RT_USING_UART

/* SECTION: Console options */
/* use console for rt_kprintf */
#define RT_USING_CONSOLE
/* the buffer size of console */
#define RT_CONSOLEBUF_SIZE	80

#define RT_USING_FINSH

#define RT_DEBUG

#endif
