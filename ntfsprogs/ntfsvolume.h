/*
 * ntfsvolume - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2002-2003 Richard Russon
 *
 * This utility will print volume information without examining any clusters.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _NTFSVOLUME_H_
#define _NTFSVOLUME_H_

#include "types.h"
#include "layout.h"

struct options {
	char		*device;	/* Device/File to work with */
	int		 quiet;		/* Less output */
	int		 verbose;	/* Extra output */
	int		 force;		/* Override common sense */
};

#endif /* _NTFSVOLUME_H_ */


