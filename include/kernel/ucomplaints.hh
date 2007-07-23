/// \file ucomplaints.hh

#ifndef UCOMPLAINTS_HH
# define UCOMPLAINTS_HH

enum UMsgType
{
  UERRORCODE = 0,
  UWARNINGCODE,
  UMSGMAX
};

/// Type of Errors
enum UErrorCode
{
  UERROR_CRITICAL = 0,
  UERROR_SYNTAX,
  UERROR_DIVISION_BY_ZERO,
  UERROR_RECEIVE_BUFFER_FULL,
  UERROR_MEMORY_OVERFLOW,
  UERROR_SEND_BUFFER_FULL,
  UERROR_RECEIVE_BUFFER_CORRUPTED,
  UERROR_MEMORY_WARNING,
  UERROR_CPU_OVERLOAD,
  UERROR_MAX // keep as last
};

/// Type of Warnings
enum UWarningCode
{
  UWARNING_MEMORY = 0,
  UWARNING_MAX // keep as last
};

const char* message (UMsgType t, int code);

// Deprecated
const char* message (UErrorCode n);
const char* message (UWarningCode n);

#endif // ! UCOMPLAINTS_HH
