#!/bin/sh
echo "#ifndef __defs_h__" >defs.h
echo "#define __defs_h__" >>defs.h

egrep 'PORT' ../queue.h|grep -v PORTA >>defs.h
grep BBSUID ../bbs.h >>defs.h

echo "#endif" >>defs.h
