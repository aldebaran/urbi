/// \file ucomplaints.hh

#ifndef UCOMPLAINTS_HH
# define UCOMPLAINTS_HH

/// Type of Errors
enum UErrorCode
{
  UERROR_CRITICAL,
  UERROR_SYNTAX,
  UERROR_DIVISION_BY_ZERO,
  UERROR_RECEIVE_BUFFER_FULL,
  UERROR_MEMORY_OVERFLOW,
  UERROR_SEND_BUFFER_FULL,
  UERROR_RECEIVE_BUFFER_CORRUPTED,
  UERROR_MEMORY_WARNING,
  UERROR_CPU_OVERLOAD
};

/// Type of Warnings
enum UWarningCode
{
  UWARNING_MEMORY
};

const char* message (UErrorCode n);

const char* message (UWarningCode n);

#endif // ! UCOMPLAINTS_HH
