![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)

Bitcoin Toolkit
===============

Bitcoin Toolkit is a collection of command line tools that allow you to generate and manipulate public and private keys, create bitcoin addresses, create vanity addresses, and interact with bitcoin nodes.

This initial release is focused mainly around public and private key manipulation, providing tools allowing the user to generate key pairs in a variety of interesting and useful ways. Users can also change the format of existing keys, such as turning on and off compression, printing its hex or decimal value, and more.

Some quick examples

The simplest way to create a new private is to use the 'btk privkey' command with the -n option:

```
$ btk privkey -n
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD
```

To generate the corresponding public key, pipe it into a ‘btk pubkey’ command like so:

```
$ echo "L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD" | btk pubkey
1EKtvvPytabULtFts1zJuXTrySVBvVFVky
```

If you want to generate a key pair in a single step, chain the previous two commands and add the '-P' option, which prints both the public and private keys separated by a single space:

```
$ btk privkey -n | btk pubkey -P
Kwr6a8nG3gsvrgwEmb2A5y31MpAyaAiWL7v7L6DGyeAnF1Y9oReu 1P1fPVgXhhHWJ3bVRrpCJwGiJowNuDvaUX
```

If you want to use a bech32 address instead, add the -B switch:

```
$ btk privkey -n | btk pubkey -PB
L3NAhbgi9mN7cCWXfPkNB9JvA12FwijxhnVyZGaobM2sRcb8nxPu bc1qx7ps2qjvclzupyry7pmg04fgpg9vlegqxdpe2e
```

In the above examples the ‘privkey' command creates the private key, and pubkey' calculates the public key from the private key. The '-n' option tells the 'privkey' command to use your local CSPRNG as a source of input when created the private key. But there are many other ways to create a private key.

Here are some more examples.
 
You can create a private key from the SHA256 hash of any given ASCII string using the -s option:

```
$ echo "brian" | btk privkey -s
L5Q16dLLLvYVuVAwEmKkGvc1B946tpnkdGwPK9M6fiLJUKEdfFZo
```

You can do the same thing with kind of binary data using the -b option. Below I can create a new private key using a photo of my daughter as input.

```
$ cat my_daughter.jpg | btk privkey -b
L33qZ2xKYegwnhyxhdeeCCcLU7cSbbG57aG5JrdXjAUqf762wZAy
```

Since a private key is essentially just a really big number, you can tell btk what number you want to use for your private key using the -d option. Here is the private key for the number 1, uncompressed (-U) and in hexadecimal format (-H).

```
$ echo "1" | btk privkey -dH
000000000000000000000000000000000000000000000000000000000000000101
```

In the above output the trailing '01' is the compression flag turned on. Everything preceding the compression flag represents is the 32-byte hexadecimal representation of 1.

If we pipe that into a pubkey command using the -h option for hex input, we can see the public key address and confirm that it has actually been used recently!

```
$ echo "1" | btk privkey -dH | btk pubkey -h
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

https://www.blockchain.com/btc/address/1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH

If you only want to generate a bitcoin address and don't care what the private key is, in most cases you can skip the 'btk privkey' command and pipe your input data straight to 'btk pubkey' like so:

```
$ echo "brian" | btk pubkey -s
15kYaHoT5wTJVshh9oZnT7D6WVDWHkAsVp
```

Or do the same with your daughter's photo:

```
cat my_daughter.jpg | btk pubkey -b
151j613Dv3Q4qwAEXvuDdnCRai2S7gdzkE
```

In the previous cases, your input is essentially your private key. Make sure they are safe and secure before sending funds to them. If you later want to spend from those addresses, you can generate the private key that can be imported in to a wallet and make those addresses spendable.

```
$ echo "brian" | btk privkey -s
L5Q16dLLLvYVuVAwEmKkGvc1B946tpnkdGwPK9M6fiLJUKEdfFZo

$ cat my_daughter.jpg | btk privkey -b
L33qZ2xKYegwnhyxhdeeCCcLU7cSbbG57aG5JrdXjAUqf762wZAy 
```

Table of Contents
-----------------

1. [Download and Install](#download-and-install)
2. [Usage](#usage)
3. [License](#license)
4. [Tips](#tips)

Download and Install
--------------------

To install this project from source, you will need to have the tools `git`,
`gcc`, and `make` to download and build it. Install them from your package
manager if they are not already installed.

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

Usage Coming Soon...

License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License. See [LICENSE](LICENSE) for
more details.
