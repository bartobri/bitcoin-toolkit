#!/usr/bin/perl

use lib './test/lib';
use Btk::TestData qw($networks $compression $iotypes $privkey $ntests);

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

## Begin Tests
for (my $i = 0; $i < $ntests; $i++)
{
	foreach my $i_network (@{$networks})
	{
		foreach my $i_comp (@{$compression})
		{
			foreach my $i_type (@{$iotypes})
			{
				my $input = $privkey->[$i]->{$i_network}->{$i_comp}->{$i_type};

				foreach my $o_network (@{$networks})
				{
					foreach my $o_comp (@{$compression})
					{
						foreach my $o_type (@{$iotypes})
						{
							my $expected = $privkey->[$i]->{$o_network}->{$o_comp}->{$o_type};
							my $output = btk_privkey_get({'from' => $i_type, 'to' => $o_type, 'network' => $o_network, 'compression' => $o_comp}, $input);
							print "$input => $output : ";
							if ($output eq $expected)
							{
								print "PASSED\n";
							}
							else
							{
								print "FAILED\n";
							}
						}
					}
				}
			}
		}
	}
}

##$result =  btk_privkey_get({'from' => 'wif', 'to' => 'wif', 'network' => 'main', 'compression' => 1 }, $privkey->[$i]->{"wif_c"});


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

	if ($params->{'network'} eq "mainnet") { $options .= "M"; }
	elsif ($params->{'network'} eq "testnet") { $options .= "T"; }

	if ($params->{'compression'} eq "uncompressed") { $options .= "U"; }
	elsif ($params->{'compression'} eq "compressed") { $options .= "C"; }

	$options .= "N";

	$result = btk_get("privkey", $options, $input);

	return $result;
}

sub btk_get
{
	my $command = shift;
	my $options = shift;
	my $input = shift;
	my $result = undef;

	$result = `printf \"$input\" | $btk_location $command $options`;
	##open (BTK, "| $btk_location $command $options") or die "Could not open a pipe to $btk_location\n";
	##print BTK $input;
	##$result = <BTK>;
	##close (BTK);

	return $result;
}

exit 0;