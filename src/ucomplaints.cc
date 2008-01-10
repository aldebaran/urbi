/// \file ucomplaints.cc

#include <cstdlib>
#include "libport/assert.hh"
#include "kernel/ucomplaints.hh"

static const char* messages[][250] =
  {
    // UErrorCode
    {
      "!!! Critical error\n",
      "!!! Syntax error\n",
      "!!! Division by zero\n",
      "!!! Receive buffer full\n",
      "!!! Out of memory\n",
      "!!! Send buffer full\n",
      "!!! Receive buffer corrupted\n",
      "!!! Memory warning\n",
    },
    // UWarningCode
    {
      "!!! Memory overflow warning\n",
    }
  };

const char*
message (UMsgType t, int code)
{
  int max = 0;

  if (t < 0 || t >= UMSGMAX)
    pabort ("unexpected case (UMsgType):" << t);

  switch (t)
  {
    case UERRORCODE:
      max = UERROR_MAX;
      break;
    case UWARNINGCODE:
      max = UWARNING_MAX;
      break;
    case UMSGMAX:
      max = -1;
      break;
  };

  if (code < 0 || code >= max)
    pabort ("unexpected case (message code):" << code);

  return messages[t][code];
}
