#!/usr/bin/perl

my $btk_location = "bin/btk";

if (!-e "test")
{
	print "Run test scripts from the bitcoin-toolkit directory\n";
	exit 1;
}
if (!-e "$btk_location")
{
	print "Compile btk by running 'make' before running test scripts.\n";
	exit 1;
}

use lib './test/lib';
use Btk::TestData '$privkey';

my $result = undef;

print $privkey->[0]->[0], "\n";
$result =  btk_privkey_get({'from' => 'wif', 'to' => 'wif', 'network' => 'main', 'compressed' => 1 }, $privkey->[0]->[0]);
print $result, "\n";

sub btk_privkey_get
{
	my $params = shift;
	my $input = shift;

	my $options = "-";
	my $result = undef;

	if ($params->{'from'} eq "wif") { $options .= "w"; }
	elsif ($params->{'from'} eq "hex") { $options .= "h"; }
	elsif ($params->{'from'} eq "dec") { $options .= "d"; }

	if ($params->{'to'} eq "wif") { $options .= "W"; }
	elsif ($params->{'to'} eq "hex") { $options .= "H"; }
	elsif ($params->{'to'} eq "dec") { $options .= "D"; }

	if ($params->{'network'} eq "main") { $options .= "M"; }
	elsif ($params->{'network'} eq "test") { $options .= "T"; }

	if ($params->{'compressed'} == 0) { $options .= "U"; }
	elsif ($params->{'compressed'} == 1) { $options .= "C"; }

	$result = btk_get("privkey", $options, $input);

	return $result;
}

sub btk_get
{
	my $command = shift;
	my $options = shift;
	my $input = shift;
	my $result = undef;

	open (BTK, "| $btk_location $command $options") or die "Could not open a pipe to $btk_location\n";
	print BTK $input;
	$result = <BTK>;
	close (BTK);

	return $result;
}

exit 0;