![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)

Bitcoin Toolkit
===============

Bitcoin Toolkit is a collection of command line tools that allow you to perform a variety of useful bitcoin-related tasks. Some of these tasks include creating and manipulating public or private keys, generating addresses (including vanity addresses) with support for various address formats, querying data on remote bitcoin nodes, and mining data from the bitcoin core database.

See the [Usage](#usage) for a quick introduction on the basics of Bitcoin Toolkit.

See the [Command Examples](#command-examples) section for more detailed information about what you can do with this application.

Once installed, run `btk help` for a detailed help menu.

Table of Contents
-----------------

1. [Download and Install](#download-and-install)
2. [Usage](#usage)
3. [Command Examples](#command-examples)
4. [License](#license)
5. [Tips](#tips)

Download and Install
--------------------
The following instructions are for unix and linux systems. Windows 10
users can install this project from within the linux subsystem.

Be sure that the following dependencies are installed on your system.
If not you can install them from your package manager.

1. libgmp
2. libgcrypt
3. libleveldb

To install them on debian systems:
```
sudo apt-get install libgmp-dev
sudo apt-get install libgcrypt20-dev
sudo apt-get install libleveldb-dev
```

You will also need `git` along with basic build tools like `make` and
`gcc`. Be sure these are also installed on your system.

Next, follow these instructions:

#### Install:
```
$ git clone https://github.com/bartobri/bitcoin-toolkit.git
$ cd ./bitcoin-toolkit
$ make
$ sudo make install
```

#### Uninstall:

```
$ sudo make uninstall
```

Usage
-----

#### Basic Usage

`btk <command> [<args>]`

Bitcoin Toolkit installs an command line tool named `btk` that handles commands and arguments much like git. The first argument is a command and is required. All following arguments are options that relate to the specified command. For example, this will create a new compressed private key in Wallet Import format:

```
$ btk privkey -n -C -W
L2L4ygLBjjYdTDnUkqLR2rwucnTbwARXyeMGJ5Svc7hScvKzmLcP
```

#### Redirecting Input

If input is required, `btk` works best when the input is redirected with a pipe or other input redirection methods. For example, this will convert the previously generated key from compressed to uncompressed format:

```
$ echo "L2L4ygLBjjYdTDnUkqLR2rwucnTbwARXyeMGJ5Svc7hScvKzmLcP" | btk privkey -w -U
5JyRvApPpkAtJisBNG2gRteMz3baHoYPRcefov7nDwXuvxBWvVR
```

#### Chaining Commands

Using input redirection methods makes it possible to chain together multiple commands which increases the utility and power of Bitcoin Toolkit. Here is the previous example with added redirection to the 'pubkey' command which ultimately prints the public key address for the private key that we started with.

```
$ echo "L2L4ygLBjjYdTDnUkqLR2rwucnTbwARXyeMGJ5Svc7hScvKzmLcP" | btk privkey -w -U | btk pubkey
1FEEedNDX5qqCEeL8FFsrjGRYU7akdVMLF
```

#### List Processing

Bitcoin Toolkit can process lists as input. Each list item must be separated by a newline character in order to be processed correctly. For example, if I have a file that contains a list of 5 uncompressed private keys:

```
$ cat keys.dat
5JShUaXdV5fuWMynP9e4hBkMMtWUBTaZs3YyitxQBffWLdSdZvt
5KfFysvDfYeMbNnnqjGxmnXnoVQe4yzr8sDTKMXPNHR49hYcChm
5JKy2J7uBDF4J2s3tnvb26ykPcz8K5jNAHqFDS7uQK2PSy5WDzt
5JUhFW78y82hQ2iGaMKdPogJxVpuwDvB3FViqaFfNqz539wccbW
5JHKyY76B9ZkWofh22pXUnCzaTNqbtNhxLwEZUQpFTxsUbXQT1Y
```

... I can pipe that list to `btk privkey` to convert all of them to compressed private keys, like so:

```
$ cat keys.dat | btk privkey -w -C
KyzReizC2wcikbbdJm8bJNSLsCM4yAKA3tG9MvKXL5exVGEUfFXE
L5Ms83jtu8fstjB7L5bTAfxuACPLcaaKsidyA1b7iwTAu3WfrsKk
KyUiGiZTJ533YBDumbhX9sPUWqq9GoszQ1HwjssUHtZ85VtsYvg4
Kz9Ef1U6Ai2VJeZ9kChDdf2WFxEUeL1hJvymfPmiPRCtpKoum7qe
KyH4kYHsqnoZ4YymdZ2mXcXw8fRPWykomGzNrKz7Yr1bnLaB4yUo
```

#### Using Help

See `btk help` for a list of available commands.

See `btk help <command>` for a list of options and more info about a command.

Command Examples
----------------

The 'privkey' command can generate and manipulate private keys. Here is how you
can create a new random private key:

```
$ btk privkey -n
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD
```

By default, keys are compressed. You can generate an uncompressed private key
like this:

```
$ btk privkey -nU
5JcNMHpGEVTuCBuhrrAqZtQS3k4PWuSwXYrbNPMq9LEDQA79Pff
```

Or you can convert a compressed private key to uncompressed:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk privkey -U
5KKLUsJPzHMvNG3AC5gSbkDDJeFdyvPDsNyvbhYER3gjftEDyuA
```

You can also convert keys (or generate new keys) to various output formats
besides WIF (Wallet Import Format), which is used by default when no output
format is specified. Below are examples where we convert the previous key to hex
and decimal:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk privkey -H
c5a85cc1e10554fdfb9eba1a6e7a188e2da7f497a87494f6b1a4b9778ab0f55c

$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk privkey -D
89403101665417317652050958343537724738876283665880345081128506555034307196252
```

You can even print private keys in their raw 32 byte state, which can be useful
if you want to store the raw data to a file or database. Then later you can read
that key data and convert it back to WIF:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk privkey -R > key.dat

$  cat key.dat | btk privkey -r
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD
```

As you can see from the previous example, multiple input formats are also
supported. In the previous example, -r tells btk to treat input as a raw binary
key. But we can also use hex and decimal as input as well. In the previous example
where we converted a key from WIF to hex and decimal, below we convert them back
to WIF.

```
$ echo "c5a85cc1e10554fdfb9eba1a6e7a188e2da7f497a87494f6b1a4b9778ab0f55c" | btk privkey -h
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD

$ echo "89403101665417317652050958343537724738876283665880345081128506555034307196252" | btk privkey -d
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD
```

We can also use plain ascii strings of any length as input. When string input
is used, the string data will be fed through a SHA256 hash algorithm to generate
a 32 byte private key. This can be useful if you want to generate a private key
from a passphrase that you've memorized.

```
$ echo "this is my secret passphrase" | btk privkey -s
L4Cp75ZzpF5AAKwN64VBTxhMfHa8bpTw6mt6uq5T5buaEEuVU1Sz
```

You can do the same thing with arbitrary length files as well. Just like with
strings, the raw data from the file will be fed through a SHA256 hash algorithm
to generate a 32 byte private key:

```
$ cat family_photo.jpg | btk privkey -b
L4SsobLicY7NLSozpjTtPiaREZrwCr9URPpbd1mB6SnwcZGirkiQ
```

The risks of using strings and binary data as input should go without saying. If
you use a string, it should be very secure and impossible to guess. If you use a
file, know that if as much as a single byte changes in that file, it will
generate a different private key.

In either case, if the data you use for input is guessable or determinable in
any way, you risk losing all of your funds.

With that said, if you are confident enough to use such inputs, then you
probably want the actual bitcoin address, not the private key, so you can store
funds at the address. Then later if you wish to access and move those funds, you
can generate the private key and import it into your perferred wallet.

To generate a bitcoin address, use the 'pubkey' command:

```
$ echo "this is my secret passphrase" | btk pubkey -s
1NRJSrg8sBua2oEBa2ExEBAsFYstdE17St
```

The pubkey command takes all the same inputs as privkey, but instead of
generating a private key, it generates a public key, usually in the form of a
bitcoin address. Below we convert a private key in WIF, hex, and decimal formats
from our previous examples and generate the public address:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk pubkey -w
1EKtvvPytabULtFts1zJuXTrySVBvVFVky

$ echo "c5a85cc1e10554fdfb9eba1a6e7a188e2da7f497a87494f6b1a4b9778ab0f55c" | btk pubkey -h
1EKtvvPytabULtFts1zJuXTrySVBvVFVky

$ echo "89403101665417317652050958343537724738876283665880345081128506555034307196252" | btk pubkey -d
1EKtvvPytabULtFts1zJuXTrySVBvVFVky
```

The pubkey command also supports multiple output formats, most notable of which
is the beck32 format. To generate a bech32 address:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk pubkey -wB
bc1qjgk83gxfsfteppluqn7g08r7qyd22clrf4nuhw
```

And while it is not particularly useful in most cases, you can print the public
key in hex format:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk pubkey -wH
030713ba7032465b53fe456d3c2ba785b320e9b873389e1ab5b71860d56f5a2563
```

You can also print both the public and private keys together which may be useful
if you interface with btk like and API and you need to store both keys:

```
echo "this is my secret passphrase" | btk pubkey -sP
L15VsN6crrGCXS4hqQZa3DF9kT6wr3ZfpDjsBQ1SAGPz6ZDFMjrF 1NRJSrg8sBua2oEBa2ExEBAsFYstdE17St
```

Bitcoin Toolkit can also create vanity addresses, although I make no claim as
to its speed, except to say that it isn't uniqely fast. But if you are willing to
wait for what could be days, it will do the job.

To create a vanity address, pipe your vanity string to btk and use the 'vanity'
command. Below we use -i for a case insensitive match, which I generally suggest
you use as well.

```
$ echo "btc" | btk vanity -i
1BtcyASYTqCFWHKjDPhz716j6jwh4SxVBw            Estimated Seconds: 149 of 422
Vanity Address Found!
Private Key: KyY3GXxNQa4Ufi7N75tuCpNS9nTMym323isZCUyw1TLQZPXzDUNk
Address:     1BtcyASYTqCFWHKjDPhz716j6jwh4SxVBw
```

You can even create a vanity address in bech32 format, although they make less
sense since the first 4 characters can't be used, and the overall character set
for bech32 addresses is much more limited.

```
$ echo "pry" | btk vanity -B
bc1qpry94dn0zz805c4rd4fjprc9rzp7cnd5chrmys    Estimated Seconds: 336 of 368
Vanity address found!
Private Key: KyqfYMuojzuHjRjH5D8XE6nYNtX9gJufNEJ6WJTSsR8Z5xNSrbkv
Address:     bc1qpry94dn0zz805c4rd4fjprc9rzp7cnd5chrmys
```

As of the time of this writing, Bitcoin Toolkit can communicate with bitcoin
nodes in a limited fashion. Which is to say, it can only query general node
info. I have plans to expand this capability in the future, but for now this is
all you can do. Still, the information gained can be very useful.

To print the general version info from a bitcoin node:

```
$ btk node -h seed.bitcoin.sipa.be
{
  "version": 70015,
  "services": {
    "bit 1": "NODE_NETWORK",
    "bit 3": "NODE_BLOOM",
    "bit 4": "NODE_WITNESS"
  },
  "timestamp": 1540926783,
  "addr_recv_services": {
  },
  "addr_recv_ip_address": "00000000000000000000ffff349072c5",
  "addr_recv_port": 49387,
  "addr_trans_services": {
    "bit 1": "NODE_NETWORK",
    "bit 3": "NODE_BLOOM",
    "bit 4": "NODE_WITNESS"
  },
  "addr_trans_ip_address": "00000000000000000000000000000000",
  "addr_trans_port": 0,
  "nonce": 3024123468520040665,
  "user_agent": "/Satoshi:0.15.1/",
  "start_height": 548011,
  "relay": true
}
```

License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License. See [LICENSE](LICENSE) for
more details.
