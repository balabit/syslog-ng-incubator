AC_DEFUN([INCUBATOR_CHECK_DEP], [if test "$enable_]$1[" != "no"; then 
  $2; 
  if test "$]$1[_found" == "no" -a "$enable_]$1[" == "yes"; then 
     AC_MSG_ERROR(Dependency for ]$1[ not found!);
  fi; 
  if test "$]$1[_found" == "no"; then
     enable_]$1[="no"
  else
     enable_]$1[="yes"
  fi
fi])dnl
