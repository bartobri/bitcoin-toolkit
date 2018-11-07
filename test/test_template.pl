#!/usr/bin/perl

if (!-e "./test")
{
	print "Run test scripts from the bitcoin-toolkit directory\n";
	exit 1;
}
if (!-e "./bin/btk")
{
	print "Compile btk by running 'make' before running test scripts.\n";
	exit 1;
}

use lib './test/lib';
use Btk::TestData '$privkey';

print $privkey->[0]->[0], "\n";

exit 0;