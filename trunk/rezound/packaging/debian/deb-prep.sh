# $Id deb-prep.sh,v 1.1 2003/02/10 14:11:35 lergnom Exp $
# Anthony Ventimiglia
#
# This script is used to prepare the Rezound cvs repository for debianization.
# It should be run from the top level directory of the cleanly checked out 
# CVS repository.
#
# It's quick and dirty.
#
# For more info read packaging/debian/README.cvs

cp bootstrap bootstrap.orig
# The new autoheader doesn't need -l and causes an error
sed -e 's/\(AUTOHEADER="autoheader\)/\12.13/g' bootstrap.orig |\
    sed -e 's/\(AUTOCONF="autoconf\)/\12.13/g' |\
    sed -e 's/\(ACLOCAL="aclocal\)/\1-1.5/g' |\
    sed -e 's/\(AUTOMAKE="automake\)/\1-1.5/g' > bootstrap

./bootstrap ;
./configure ;

make dist ;

mv bootstrap.orig bootstrap
tar -zxvf rezound*.tar.gz


