/*
 * File      : idle.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-23     Bernard      the first version
 * 2010-11-10     Bernard      add cleanup callback function in thread exit.
 */

#include <rthw.h>
#include <rtthread.h>
#include "kservice.h"

#ifndef IDLE_THREAD_STACK_SIZE
#if defined (RT_USING_HOOK) || defined(RT_USING_HEAP)
#define IDLE_THREAD_STACK_SIZE	256
#else
#define IDLE_THREAD_STACK_SIZE	128
#endif
#endif

static struct rt_thread idle;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_thread_stack[IDLE_THREAD_STACK_SIZE];

extern rt_list_t rt_thread_defunct;

#ifdef RT_USING_HOOK
/**
 * @addtogroup Hook
 */
/*@{*/

static void (*rt_thread_idle_hook)();

/**
 * This function will set a hook function to idle thread loop.
 *
 * @param hook the specified hook function
 *
 * @note the hook function must be simple and never be blocked or suspend.
 */
void rt_thread_idle_sethook(void (*hook)())
{
	rt_thread_idle_hook = hook;
}
/*@}*/
#endif

/**
 * @ingroup Thread
 *
 * This function will perform system background job when system idle.
 */
void rt_thread_idle_excute(void)
{
	RT_DEBUG_NOT_IN_INTERRUPT;

	/* check the defunct thread list */
	if (!rt_list_isempty(&rt_thread_defunct))
	{
		rt_base_t lock;
		rt_thread_t thread;
#ifdef RT_USING_MODULE
		rt_module_t module = RT_NULL;
#endif
		/* disable interrupt */
		lock = rt_hw_interrupt_disable();

		/* re-check whether list is empty */
		if (!rt_list_isempty(&rt_thread_defunct))
		{
			/* get defunct thread */
			thread = rt_list_entry(rt_thread_defunct.next, struct rt_thread, tlist);
#ifdef RT_USING_MODULE
			/* get thread's parent module */
			module = (rt_module_t)thread->module_id;

			/* if the thread is module's main thread */
			if (module != RT_NULL && module->module_thread == thread)
			{
				/* detach module's main thread */
				module->module_thread = RT_NULL;
			}
#endif
			/* remove defunct thread */
			rt_list_remove(&(thread->tlist));
			/* invoke thread cleanup */
			if (thread->cleanup != RT_NULL) thread->cleanup(thread);

			/* if it's a system object, not delete it */
			if (rt_object_is_systemobject((rt_object_t)thread) == RT_EOK)
			{
				/* enable interrupt */
				rt_hw_interrupt_enable(lock);
				return;
			}
		}
		else
		{
			/* enable interrupt */
			rt_hw_interrupt_enable(lock);

			/* may the defunct thread list is removed by others, just return */
			return;
		}

		/* enable interrupt */
		rt_hw_interrupt_enable(lock);

#ifdef RT_USING_HEAP
#if defined(RT_USING_MODULE) && defined(RT_USING_SLAB)
		/* the thread belongs to an application module */
		if (thread->flags & RT_OBJECT_FLAG_MODULE)
			rt_module_free((rt_module_t)thread->module_id, thread->stack_addr);
		else
#endif
		/* release thread's stack */
		rt_free(thread->stack_addr);
		/* delete thread object */
		rt_object_delete((rt_object_t)thread);
#endif

#ifdef RT_USING_MODULE
		if (module != RT_NULL)
		{	
			/* if sub thread list and main thread are all empty */
			if ((module->module_thread == RT_NULL) &&
				rt_list_isempty(&module->module_object[RT_Object_Class_Thread].object_list) )
			{
				module->nref --;
			}
		}

		/* unload module */
		if (module->nref == 0)	rt_module_unload(module);
#endif
	}
}

static void rt_thread_idle_entry(void *parameter)
{
	while (1)
	{
		#ifdef RT_USING_HOOK
		if (rt_thread_idle_hook != RT_NULL)
			rt_thread_idle_hook();
		#endif

		rt_thread_idle_excute();
	}
}

/**
 * @ingroup SymstemInit
 *
 * This function will initialize idle thread, then start it.
 *
 * @note this function must be invoked when system init.
 */
void rt_thread_idle_init(void)
{
	/* init thread */
	rt_thread_init(&idle,
		"tidle",
		rt_thread_idle_entry, RT_NULL,
		&rt_thread_stack[0], sizeof(rt_thread_stack),
		RT_THREAD_PRIORITY_MAX - 1, 32);

	/* startup */
	rt_thread_startup(&idle);
}
