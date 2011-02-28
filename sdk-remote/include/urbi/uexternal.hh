/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UEXTERNAL_HH
# define URBI_UEXTERNAL_HH

namespace urbi
{

  // Warning, the values are replicated in uobject.u, keep in sync.
  enum USystemExternalMessage
  {
    UEM_EVALFUNCTION, // K->R
    UEM_ASSIGNVALUE,  // K<->R
    UEM_EMITEVENT,    // K<->R
    UEM_ENDEVENT,     // K->R
    UEM_NEW,          // K->R
    UEM_DELETE,       // K->R
    UEM_INIT,   // Internal, force loading of all uobjects
    UEM_TIMER,  // Internal timer messages
    UEM_NORTP,  // K->R Disable RTP for this connection
    UEM_SETRTP, // K->R [varname, state(0:off 1:on)]: Set rtp state.
    UEM_REPLY,  // R->K  Function call return value from a remote
    UEM_EVAL,   // R->K  Request to evaluate the string argument
    UEM_SETLOCAL, // K->R(varname, enable) mark all uvars varname as local
  };

  static const std::string externalModuleTag = "__ExternalMessage__";

} // namespace urbi

#endif
