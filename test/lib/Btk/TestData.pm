package Btk::TestData;

BEGIN
{
	use Exporter();
	@ISA = qw(Exporter);
	@EXPORT_OK = qw($privkey $networks $compression $iotypes $ntests);
}

$iotypes = ["wif", "hex", "dec"];
$networks = ["mainnet", "testnet", "na"];
$compression = ["compressed", "uncompressed", "na"];
$ntests = 1;

$privkey = [
	{
		"mainnet" => {
			"compressed" => {
				"wif" => "L2MVy8izFg8mDDbLPh67BzBRcYukUDSkB5DbXkCJHzoMj1kRE4hZ",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0701",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"uncompressed" => {
				"wif" => "5JykiidFLBgNLzEDYsmf77cSWoenC9iBEcPntNZvJu4hwvxKAcF",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0700",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"na" => {
				"wif" => "L2MVy8izFg8mDDbLPh67BzBRcYukUDSkB5DbXkCJHzoMj1kRE4hZ",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a07",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			}
		},
		"testnet" => {
			"compressed" => {
				"wif" => "cSiVS3iqgjq2Nf4bn6uEZJgVEnDA8fYSF7N4eAeoo7TMykprVS73",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0701",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"uncompressed" => {
				"wif" => "92kPJTSnvQkWK3jWBDfZyiAQAU1VMKFNaZFjxzvRedokivEEB4T",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0700",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"na" => {
				"wif" => "cSiVS3iqgjq2Nf4bn6uEZJgVEnDA8fYSF7N4eAeoo7TMykprVS73",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a07",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			}
		},
		"na" => {
			"compressed" => {
				"wif" => "L2MVy8izFg8mDDbLPh67BzBRcYukUDSkB5DbXkCJHzoMj1kRE4hZ",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0701",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"uncompressed" => {
				"wif" => "5JykiidFLBgNLzEDYsmf77cSWoenC9iBEcPntNZvJu4hwvxKAcF",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a0700",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			},
			"na" => {
				"wif" => "L2MVy8izFg8mDDbLPh67BzBRcYukUDSkB5DbXkCJHzoMj1kRE4hZ",
				"hex" => "9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a07",
				"dec" => "69291675717989167975973302432652508210900605924011867510192277652346948246023",
			}
		}
	},
	##{
	##	"wif_c" => "L1qptfDJigmyHbfCtv5LLzvhKKgxguMphbM6vKMGnpYkNSfh6R5Z",
	##	"wif_u" => "5Js2ngvYCiBeDpjBrNYLnmPb4FCmnk8SJokXLnPie7hGiapMqUQ",
	##	"hex" => "89edc84d96c4c188aa4e3a9505048b59591edd5c4c8ebab52f76d44fc0bb7572",
	##	"dec" => "62386985451323008247109978397815028354118254627114116447645500136332742849906",
	##	"tst_c" => "cSCpMaDA9kUET38UHKtTiKRkwYzNMMTWmdVa2jonHwCkdBkWtJwW",
	##	"tst_u" => "92dfNRk5nwFnBtEUUiSFfMwYhuZUwufdekcURQkDyrSKVdbepy3",
	##},
];

return 1;