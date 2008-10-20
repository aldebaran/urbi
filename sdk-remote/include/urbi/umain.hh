
/*! \file urbi/usyncclient.hh
****************************************************************************
*
* Definition of the URBI interface class
*
* Copyright (C) 2004, 2006, 2007 Jean-Christophe Baillie.  All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#ifndef URBI_UMAIN_HH
# define URBI_UMAIN_HH

# include <urbi/uobject.hh>

#  define UMAIN()				\
  int						\
  main(int argc, const char *argv[])		\
  {						\
    urbi::main(argc, argv, true);		\
  }

#endif /* !URBI_UMAIN_HH */
