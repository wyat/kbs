dnl Copy from acinclude.m4 of php.
AC_DEFUN([BBS_PROG_SENDMAIL],[
BBS_ALT_PATH=/usr/bin:/usr/sbin:/usr/etc:/etc:/usr/ucblib:/usr/lib
AC_PATH_PROG(PROG_SENDMAIL, sendmail,[], $PATH:$BBS_ALT_PATH)
if test -n "$PROG_SENDMAIL"; then
  AC_DEFINE(HAVE_SENDMAIL,1,[whether you have sendmail])
  AC_DEFINE_UNQUOTED(OWNSENDMAIL,"$PROG_SENDMAIL",[path of sendmail program])
else
  AC_MSG_ERROR(not found)
fi
])

AC_DEFUN([BBS_LIB_ICONV],[
  ICONV_DIR=
  for i in /usr/local /usr ; do
    if test -f $i/include/iconv.h; then
      ICONV_DIR=$i
      ICONV_INC=$i/include
    fi
  done
  if test -z "$ICONV_DIR"; then
    AC_MSG_ERROR(The iconv library not found)
  fi
  SAVE_LIBS=$LIBS
  LIBS="$SAVE_LIBS -L$ICONV_DIR/lib"
  LIBICONV_CFLAGS=
  LIBICONV_FLAGS=
  AC_CHECK_LIB(iconv, iconv_open, [
    AC_DEFINE(HAVE_LIBICONV,1,[ ])
	LIBICONV_CFLAGS="-I$ICONV_INC"
	LIBICONV_FLAGS="-L$ICONV_DIR/lib -liconv"
  ],[
    LIBICONV_NOT_FOUND="yes"
  ])
  if test "$LIBICONV_NOT_FOUND" = "yes"; then
    AC_CHECK_LIB(c, iconv_open, [
      AC_DEFINE(HAVE_LIBICONV,1,[ ])
    ],[
      AC_MSG_ERROR(The iconv library not found)
    ])
  fi
  LIBS=$SAVE_LIBS
  AC_SUBST(LIBICONV_CFLAGS)
  AC_SUBST(LIBICONV_FLAGS)
])
