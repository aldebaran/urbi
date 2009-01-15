/*! \file urbi/uabstractclient.hxx
 ****************************************************************************
 *
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Jean-Christophe Baillie.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 **********************************************************************/

#ifndef URBI_UABSTRACTCLIENT_HXX
# define URBI_UABSTRACTCLIENT_HXX

namespace urbi
{

  inline
  const std::string&
  UAbstractClient::getServerName() const
  {
    return host_;
  }

  /// Return the server port.
  inline
  unsigned
  UAbstractClient::getServerPort() const
  {
    return port_;
  }

  inline
  int
  UAbstractClient::kernelMajor() const
  {
    return kernelMajor_;
  }

  inline
  int
  UAbstractClient::kernelMinor() const
  {
    return kernelMinor_;
  }

  inline
  const std::string&
  UAbstractClient::kernelVersion() const
  {
    return kernelVersion_;
  }

} // namespace urbi

#endif // URBI_UABSTRACTCLIENT_HXX
