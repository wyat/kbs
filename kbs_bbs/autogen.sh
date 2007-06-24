#!/bin/sh

# quick & dirty fix for CVS. need better solution. 20051004 atppp
mkdir -p innbbsd; touch innbbsd/Makefile.in;

aclocal; libtoolize -c --force; autoheader; automake -a; autoconf
