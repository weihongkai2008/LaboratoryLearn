/*
* Copyright 2015 WeiHongkai
*/

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

int ApuePrint(char *fmt, ...);
size_t ProcessFormat(char *buffer, char *fmt, va_list ap);
char *TransString(char *str, char *string);
char *TransInteger(char *str, int num);
char *TransFloat(char *str, float flt);

// find the position of digit in the array digit.
int
Divide(int number) {
  return number % 10;
}

// The string data converts into the format-string that will be appended to str
char *
TransString(char *str, char *para_string) {
  size_t len_of_string;

  len_of_string = strlen(para_string);
  for (int i = 0; i < len_of_string; i++) {
    *str++ = *para_string++;
  }
  return str;
}

// The integer converts into the format-string that will be appended to str
char *
TransInteger(char *str, int num) {
  const char *digit = "0123456789";
  char tmp_num[56];
  int i = 0;

  if (num < 0) {
    num = -num;
    *str++ = '-';
  }

  if (num == 0) {
    tmp_num[i++] = '0';
  }
  else {
    while (num != 0) {
      tmp_num[i++] = digit[Divide(num)];
      num = num / 10;
    }
  }
  while (i-- > 0)
    *str++ = tmp_num[i];
  return str;
}

// The decimal converts into the format-string that will be appended to str
char *
TransFloat(char *str, float flt) {
  const char *digit = "0123456789";
  char tmp_float[32];
  int i = 0;
  int integer, decimal;

  if (flt < 0) {   // the char don't support the negative number
    flt = -flt;
    *str++ = '-';
  }
  integer = (int) flt;
  decimal = (flt - integer) * 1000000;  // define the accuracy
  if (integer == 0)
    *str++ = digit[0];
  while (integer != 0) {
    tmp_float[i++] = digit[Divide(integer)];
    integer = integer / 10;
  }
  while (i-- > 0)    // from the end to the start
    *str++ = tmp_float[i];
  *str++ = '.';
  i = 0;
  while (decimal != 0) {
    tmp_float[i++] = digit[Divide(decimal)];
    decimal = decimal / 10;
  }
  while (i-- > 0)
    *str++ = tmp_float[i];

  return str;
}

// make the input data converts into the format-string in different paras
size_t
ProcessFormat(char *buffer, char *fmt, va_list ap) {
  char *str;

  for (str = buffer; *fmt; ++fmt) {
    if (*fmt != '%') {
      *str++ = *fmt;
      continue;
    }
    ++fmt;
    switch (*fmt) {
      case 's':
        str = TransString(str, va_arg(ap, char *));
        break;

      case 'd':
        str = TransInteger(str, va_arg(ap, int));
        break;

      case 'f':
        str =
            TransFloat(str, (float) va_arg(ap, double));  // Don't input the float type data!!!the define don't support
        break;

      default:
        if (*fmt != '%')
          *str++ = '%';
        if (*fmt)
          *str++ = *fmt;
        else
          --fmt;
        break;
    }
  }

  *str = '\0';
  return (str - buffer);
}

int
ApuePrint(char *fmt, ...) {
  const size_t kSizeOfBuffer = 1024;
  char buffer[kSizeOfBuffer];
  size_t size_of_print;
  va_list ap;

  va_start(ap, fmt);
  size_of_print = ProcessFormat(buffer, fmt, ap);
  va_end(ap);
  write(1, buffer, size_of_print);

  return 0;
}

int
main() {
  char *temp1 = "helloworld";
  int temp2 = 237;
  double temp3 = 1.1236;

  ApuePrint("1)This is the string :%s  ", temp1);
  ApuePrint("2)This is the integer number :%d  ", temp2);
  ApuePrint("3)This is the float number :%f  ", temp3);
  ApuePrint("4)This is the three-format-string:%s %d %f", temp1, temp2, temp3);

  return 0;
}
