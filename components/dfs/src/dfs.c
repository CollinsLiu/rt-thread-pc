/*
 * File      : dfs.c
 * This file is part of Device File System in RT-Thread RTOS
 * COPYRIGHT (C) 2004-2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2005-02-22     Bernard      The first version.
 * 2010-07-16
 */

#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_file.h>

#define NO_WORKING_DIR	"system does not support working dir\n"

/* Global variables */
const struct dfs_filesystem_operation* filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX];
struct dfs_filesystem filesystem_table[DFS_FILESYSTEMS_MAX];

/* device filesystem lock */
static struct rt_mutex fslock;

#ifdef DFS_USING_WORKDIR
char working_directory[DFS_PATH_MAX] = {"/"};
#endif

#ifdef DFS_USING_STDIO
struct dfs_fd fd_table[3 + DFS_FD_MAX];
#else
struct dfs_fd fd_table[DFS_FD_MAX];
#endif

/**
 * @addtogroup DFS
 */
/*@{*/

/**
 * this function will initialize device file system.
 */
void dfs_init()
{
	/* clear filesystem operations table */
	rt_memset(filesystem_operation_table, 0, sizeof(filesystem_operation_table));
	/* clear filesystem table */
	rt_memset(filesystem_table, 0, sizeof(filesystem_table));
	/* clean fd table */
	rt_memset(fd_table, 0, sizeof(fd_table));

	/* create device filesystem lock */
	rt_mutex_init(&fslock, "fslock", RT_IPC_FLAG_FIFO);

#ifdef DFS_USING_WORKDIR
	/* set current working directory */
	rt_memset(working_directory, 0, sizeof(working_directory));
	working_directory[0] = '/';
#endif
}

/**
 * this function will lock device file system.
 *
 * @note please don't invoke it on ISR.
 */
void dfs_lock()
{
	rt_err_t result;

	result = rt_mutex_take(&fslock, RT_WAITING_FOREVER);
	if (result != RT_EOK)
	{
		RT_ASSERT(0);
	}
}

/**
 * this function will lock device file system.
 *
 * @note please don't invoke it on ISR.
 */
void dfs_unlock()
{
	rt_mutex_release(&fslock);
}

/**
 * @ingroup Fd
 * This function will allocate a file descriptor.
 *
 * @return -1 on failed or the allocated file descriptor.
 */
int fd_new(void)
{
	struct dfs_fd* d;
	int idx;

	/* lock filesystem */
	dfs_lock();

	/* find an empty fd entry */
#ifdef DFS_USING_STDIO
	for (idx = 3; idx < DFS_FD_MAX + 3 && fd_table[idx].ref_count > 0; idx++);
#else
	for (idx = 0; idx < DFS_FD_MAX && fd_table[idx].ref_count > 0; idx++);
#endif

	/* can't find an empty fd entry */
#ifdef DFS_USING_STDIO
	if (idx == DFS_FD_MAX + 3)
#else
	if (idx == DFS_FD_MAX)
#endif
	{
		idx = -1;
		goto __result;
	}

	d = &(fd_table[idx]);
	d->ref_count = 1;

__result:
	dfs_unlock();
	return idx;
}

/**
 * @ingroup Fd
 *
 * This function will return a file descriptor structure according to file
 * descriptor.
 *
 * @return NULL on on this file descriptor or the file descriptor structure
 * pointer.
 */
struct dfs_fd* fd_get(int fd)
{
	struct dfs_fd* d;

#ifdef DFS_USING_STDIO
	if ( fd < 3 || fd > DFS_FD_MAX + 3) return RT_NULL;
#else
	if ( fd < 0 || fd > DFS_FD_MAX ) return RT_NULL;
#endif

	dfs_lock();
	d = &fd_table[fd];

	/* increase the reference count */
	d->ref_count ++;
	dfs_unlock();

	return d;
}

/**
 * @ingroup Fd
 *
 * This function will put the file descriptor.
 */
void fd_put(struct dfs_fd* fd)
{
	dfs_lock();
	fd->ref_count --;

	/* clear this fd entry */
	if ( fd->ref_count == 0 )
	{
		rt_memset(fd, 0, sizeof(struct dfs_fd));
	}
	dfs_unlock();
};

/** 
 * @ingroup Fd
 *
 * This function will return whether this file has been opend.
 * 
 * @param pathname the file path name.
 *
 * @return 0 on file has been open successfully, -1 on open failed.
 */
int fd_is_open(const char* pathname)
{
	char *fullpath;
	unsigned int index;
	struct dfs_filesystem* fs;
	struct dfs_fd* fd;

	fullpath = dfs_normalize_path(RT_NULL, pathname);
	if (fullpath != RT_NULL)
	{
		char *mountpath;
		fs = dfs_filesystem_lookup(fullpath);
		if (fs == RT_NULL)
		{
			/* can't find mounted file system */
			rt_free(fullpath);
			return -1;
		}

		/* get file path name under mounted file system */
		if (fs->path[0] == '/' && fs->path[1] == '\0')
			mountpath = fullpath;
		else mountpath = fullpath + strlen(fs->path);

		dfs_lock();
		for (index = 0; index < DFS_FD_MAX; index++)
		{
			fd = &(fd_table[index]);
			if (fd->fs == RT_NULL) continue;

			if (fd->fs == fs &&
				strcmp(fd->path, mountpath) == 0)
			{
				/* found file in file descriptor table */
				rt_free(fullpath);
				dfs_unlock();
				return 0;
			}
		}
		dfs_unlock();

		rt_free(fullpath);
	}

	return -1;
}

/**
 * this function will return a sub-path name under directory.
 *
 * @param directory the parent directory.
 * @param filename the filename.
 *
 * @return the subdir pointer in filename
 */
const char* dfs_subdir(const char* directory, const char* filename)
{
	const char* dir;

	if (strlen(directory) == strlen(filename)) /* it's a same path */
		return RT_NULL;

	dir = filename + strlen(directory);
	if ((*dir != '/') && (dir != filename))
	{
		dir --;
	}
	return dir;
}

/** 
 * this function will normalize a path according to specified parent directory and file name.
 *
 * @param directory the parent path
 * @param filename the file name
 *
 * @return the built full file path (absoluted path)
 */
char* dfs_normalize_path(const char* directory, const char* filename)
{
	char *fullpath;
	char *dst0, *dst, *src;

	/* check parameters */
	RT_ASSERT(filename != RT_NULL);

#ifdef DFS_USING_WORKDIR
	if (directory == NULL) /* shall use working directory */
		directory = &working_directory[0];
#else
	if ((directory == NULL) && (filename[0] != '/'))
	{
		rt_kprintf(NO_WORKING_DIR);
		return RT_NULL;
	}
#endif

	if (filename[0] != '/') /* it's a absolute path, use it directly */
	{
		fullpath = rt_malloc(strlen(directory) + strlen(filename) + 2);

		/* join path and file name */
		rt_snprintf(fullpath, strlen(directory) + strlen(filename) + 2, 
			"%s/%s", directory, filename);
	}
	else
	{
		fullpath = rt_strdup(filename); /* copy string */
	}

	src = fullpath;
	dst = fullpath;
	
	dst0 = dst;
	while (1)
	{
		char c = *src;

		 if (c == '.')
		 {
			 if (!src[1]) src ++; /* '.' and ends */
			 else if (src[1] == '/')
			 {
				 /* './' case */
				 src += 2;

				 while ((*src == '/') && (*src != '\0')) src ++;
				 continue;
			 }
			 else if (src[1] == '.')
			 {
				 if (!src[2])
				 {
					/* '..' and ends case */
					 src += 2;
					 goto up_one;
				 }
				 else if (src[2] == '/')
				 {
					/* '../' case */
					 src += 3;

					 while ((*src == '/') && (*src != '\0')) src ++;
					 goto up_one;
				 }
			 }
		 }

		 /* copy up the next '/' and erase all '/' */
		 while ((c = *src++) != '\0' && c != '/') *dst ++ = c;

		 if (c == '/')
		 {
			 *dst ++ = '/';
			 while (c == '/') c = *src++;

			 src --;
		 }
		 else if (!c) break;

		 continue;

up_one:
		dst --;
		if (dst < dst0) { rt_free(fullpath); return NULL;}
		while (dst0 < dst && dst[-1] != '/') dst --;
	}

	*dst = '\0';

	/* remove '/' in the end of path if exist */
	dst --;
	if ((dst != fullpath) && (*dst == '/')) *dst = '\0';

	return fullpath;
}
/*@}*/

