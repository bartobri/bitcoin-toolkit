package Btk::TestData;

BEGIN
{
	use Exporter();
	@ISA = qw(Exporter);
	@EXPORT_OK = qw($privkey);
}

## Compressed WIF
## Uncompressed WIF
## Hex
## Decimal
## Test Compressed
## Test Uncompressed
$privkey = [
	[
		"L2MVy8izFg8mDDbLPh67BzBRcYukUDSkB5DbXkCJHzoMj1kRE4hZ",
		"5JykiidFLBgNLzEDYsmf77cSWoenC9iBEcPntNZvJu4hwvxKAcF",
		"9931b2d9a562a8cb9d20671d77e126c61759be440376af5a5eac270cefa75a07",
		"69291675717989167975973302432652508210900605924011867510192277652346948246023",
		"cSiVS3iqgjq2Nf4bn6uEZJgVEnDA8fYSF7N4eAeoo7TMykprVS73",
		"92kPJTSnvQkWK3jWBDfZyiAQAU1VMKFNaZFjxzvRedokivEEB4T",
	],
	[
		"L1qptfDJigmyHbfCtv5LLzvhKKgxguMphbM6vKMGnpYkNSfh6R5Z",
		"5Js2ngvYCiBeDpjBrNYLnmPb4FCmnk8SJokXLnPie7hGiapMqUQ",
		"89edc84d96c4c188aa4e3a9505048b59591edd5c4c8ebab52f76d44fc0bb7572",
		"62386985451323008247109978397815028354118254627114116447645500136332742849906",
		"cSCpMaDA9kUET38UHKtTiKRkwYzNMMTWmdVa2jonHwCkdBkWtJwW",
		"92dfNRk5nwFnBtEUUiSFfMwYhuZUwufdekcURQkDyrSKVdbepy3",
	],
];

return 1;