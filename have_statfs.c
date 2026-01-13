/*
 * have_statfs - Determine if we have statfs()
 *
 * Copyright (C) 2023,2026  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   2023/03/06 00:51:53
 * File existed as early as:    2023
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * usage:
 *      have_statfs
 *
 * Not all systems have the statfs() function, so this may not
 * compile on your system.
 *
 * This prog outputs several defines:
 *
 *      HAVE_STATFS
 *              defined ==> use statfs()
 *              undefined ==> do not call or cannot call statfs()
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * conditional <system> head includes
 */
#include "have_sys_vfs.h"
#if defined(HAVE_SYS_VFS_H)
#  include <sys/vfs.h>
#endif

#include "have_sys_param.h"
#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

#include "have_sys_mount.h"
#if defined(HAVE_SYS_MOUNT_H)
#  include <sys/mount.h>
#endif

/*
 * calc local src includes
 */

#include "banned.h" /* include after all other includes */

int
main(void)
{
#if defined(HAVE_NO_STATFS)

    printf("#undef HAVE_STATFS /* no */\n");

#else

    struct statfs statfs_dot; /* usage stat of "." */

    (void)statfs(".", &statfs_dot);

    printf("#define HAVE_STATFS /* yes */\n");

#endif

    /* exit(0); */
    return 0;
}
