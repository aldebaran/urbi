/// \file ucomplaints.cc

#include <cstdlib>
#include "ucomplaints.hh"

const char*
message (UErrorCode n)
{
  switch (n)
  {
    case UERROR_CRITICAL:
      return "!!! Critical error\n";
    case UERROR_SYNTAX:
      return "!!! Syntax error\n";
    case UERROR_DIVISION_BY_ZERO:
      return "!!! Division by zero\n";
    case UERROR_RECEIVE_BUFFER_FULL:
      return "!!! Receive buffer full\n";
    case UERROR_MEMORY_OVERFLOW:
      return "!!! Out of memory\n";
    case UERROR_SEND_BUFFER_FULL:
      return "!!! Send buffer full\n";
    case UERROR_CPU_OVERLOAD:
      return "!!! CPU Overload\n";
    case UERROR_RECEIVE_BUFFER_CORRUPTED:
      return "!!! Receive buffer corrupted\n";
    case UERROR_MEMORY_WARNING:
      return "!!! Memory warning\n";
  }
  pabort ("unexpected case:" << n);
}

const char*
message (UWarningCode n)
{
  switch (n)
  {
    case UWARNING_MEMORY:
      return "!!! Memory overflow warning\n";
  }
  pabort ("unexpected case: " << n);
}
