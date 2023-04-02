import sys
import json
import unittest
from .btk import BTK

inputs = [
    {
        "wif": "Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu",
        "wif_u": "5JbtoEnCt6yAWCUvKwKYeCitigTV5qzTHtwHKa7Lhuk4sYDnTpP",
        "wif_test": "cR415zpNsd3A7K9VkzZz2XExuCTJcqLH7cP49PbVUgT6LaJiWb1Q",
        "wif_test_u": "92NXNybkUL3JUFzCxHDTWoGrNLpCF1XedqoEQCTr3eV7ebgGHp5",
        "hex": "039927c7c042cfcd668b6e1abbb83b60591966047c7de2b15f5154982e5cf10fa6",
        "hex_u": "049927c7c042cfcd668b6e1abbb83b60591966047c7de2b15f5154982e5cf10fa67487f3b6e3b2c4b5ab067b2cc910270276ddef5ad8392c97a9c6039d1467bedf",

        "p2pkh": "13UbdKa4jR8V7cQbpGaq3nK8K5ocrHKCFo",
        "p2pkh_u": "1LeS39jXoq5qahHJU5dWvW7EuurHCLnn32",
        "p2pkh_test": "mhzYvNf3YSZjtitDXqZCshXTB5QKmnny9F",
        "p2pkh_test_u": "n1APLCpWcrX6MokvBebtkRKZmuSz8qhrsq",
        "bech32": "bc1qrv5xx9gjzjpqawkvu7nfx4q69hvx0hw2cfydnh",
        "bech32_test": "tb1qrv5xx9gjzjpqawkvu7nfx4q69hvx0hw2j0l7gy",
    }
]

class Address(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Address)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("address")

    def test_0010(self):
        print("address")

    def io_test(self, opts, input, output, input_json=True, output_json=True):

        for input_group in inputs:
            self.btk.reset()

            if (input):
                if (input_json):
                    self.btk.set_input(f"[\n\"{input_group[input]}\"\n]")
                else:
                    self.btk.set_input(input_group[input])

            for opt in opts:
                self.btk.arg(opt)

            out = self.btk.run()

            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)

            out_text = out.stdout

            self.assertTrue(out_text)

            if (output_json):
                self.assertTrue(out_text[0] == "[" or out_text[0] == "{")

                out_json = json.loads(out_text)
                out_text = out_json[0]
            else:
                self.assertTrue(out_text[0] != "[" and out_text[0] != "{")

            if (output):
                self.assertTrue(out_text == input_group[output])

    ################
    ## Output Format
    ################

    def test_0010(self):
        self.io_test(opts=["-w", "-J"], input="wif", output=None)

    def test_0020(self):
        self.io_test(opts=["-w", "--out-format=json"], input="wif", output=None)

    def test_0030(self):
        self.io_test(opts=["-w", "-L"], input="wif", output=None, output_json=False)

    def test_0040(self):
        self.io_test(opts=["-w", "--out-format=list"], input="wif", output=None, output_json=False)

    ################
    ## Input Format
    ################

    def test_0050(self):
        self.io_test(opts=["-w", "-l"], input="wif", output=None, input_json=False)

    def test_0060(self):
        self.io_test(opts=["-w", "--in-format=list"], input="wif", output=None, input_json=False)

    def test_0070(self):
        self.io_test(opts=["-w", "-j"], input="wif", output=None)

    def test_0080(self):
        self.io_test(opts=["-w", "--in-format=json"], input="wif", output=None)

    ###########
    ## WIF
    ###########

    def test_0090(self):
        self.io_test(opts=[], input="wif", output="p2pkh")

    def test_0100(self):
        self.io_test(opts=["-w"], input="wif", output="p2pkh")

    def test_0110(self):
        self.io_test(opts=["--in-type=wif"], input="wif", output="p2pkh")

    def test_0120(self):
        self.io_test(opts=["-w", "--p2pkh"], input="wif", output="p2pkh")

    def test_0130(self):
        self.io_test(opts=["-w", "--bech32"], input="wif", output="bech32")

    ####################
    ## WIF Uncompressed
    ####################

    def test_0140(self):
        self.io_test(opts=[], input="wif_u", output="p2pkh_u")

    def test_0150(self):
        self.io_test(opts=["-w"], input="wif_u", output="p2pkh_u")

    def test_0160(self):
        self.io_test(opts=["--in-type=wif"], input="wif_u", output="p2pkh_u")

    def test_0170(self):
        self.io_test(opts=["-w", "--p2pkh"], input="wif_u", output="p2pkh_u")

    ###########
    ## WIF Test
    ###########

    def test_0180(self):
        self.io_test(opts=[], input="wif_test", output="p2pkh_test")

    def test_0190(self):
        self.io_test(opts=["-w"], input="wif_test", output="p2pkh_test")

    def test_0200(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test", output="p2pkh_test")

    def test_0210(self):
        self.io_test(opts=["-w", "--p2pkh"], input="wif_test", output="p2pkh_test")

    def test_0220(self):
        self.io_test(opts=["-w", "--bech32"], input="wif_test", output="bech32_test")

    #########################
    ## WIF Test Uncompressed
    #########################

    def test_0230(self):
        self.io_test(opts=[], input="wif_test_u", output="p2pkh_test_u")

    def test_0240(self):
        self.io_test(opts=["-w"], input="wif_test_u", output="p2pkh_test_u")

    def test_0250(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test_u", output="p2pkh_test_u")

    def test_0260(self):
        self.io_test(opts=["-w", "--p2pkh"], input="wif_test_u", output="p2pkh_test_u")

    ###########
    ## HEX
    ###########

    def test_0270(self):
        self.io_test(opts=[], input="hex", output="p2pkh")

    def test_0280(self):
        self.io_test(opts=["-x"], input="hex", output="p2pkh")

    def test_0290(self):
        self.io_test(opts=["--in-type=hex"], input="hex", output="p2pkh")

    def test_0300(self):
        self.io_test(opts=["-x", "--p2pkh"], input="hex", output="p2pkh")

    def test_0310(self):
        self.io_test(opts=["-x", "--bech32"], input="hex", output="bech32")

    ####################
    ## HEX Uncompressed
    ####################

    def test_0320(self):
        self.io_test(opts=[], input="hex_u", output="p2pkh_u")

    def test_0330(self):
        self.io_test(opts=["-x"], input="hex_u", output="p2pkh_u")

    def test_0340(self):
        self.io_test(opts=["--in-type=hex"], input="hex_u", output="p2pkh_u")

    def test_0350(self):
        self.io_test(opts=["-x", "--p2pkh"], input="hex_u", output="p2pkh_u")

    ####################
    ## Grep
    ####################

    def test_0360(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("--p2pkh")
        self.btk.arg("--grep=\"13UbdK\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)

        out_text = out.stdout

        self.assertTrue(out_text)

    def test_0370(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("--p2pkh")
        self.btk.arg("--grep=\"00000000\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertFalse(out.stdout)

    ###############
    ## Match Tests
    ###############

    def test_0380(self):

        progress_char = ["-", "\\", "|", "/"]

        for i in range(100):

            print(progress_char[i % 4], end='')
            sys.stdout.flush()

            # create privkey
            self.btk.reset("privkey")
            self.btk.arg("--create")
            self.btk.arg("-W")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            privkey_json = json.loads(out_text)
            self.assertTrue(privkey_json[0])

            # create pubkey
            self.btk.reset("address")
            self.btk.set_input(privkey_json[0])
            self.btk.arg("-w")
            self.btk.arg("--bech32")
            self.btk.arg("--p2pkh")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            address_json1 = json.loads(out_text)
            self.assertTrue(address_json1[0])
            self.assertTrue(address_json1[1])

            # create pubkey
            self.btk.reset()
            self.btk.set_input(privkey_json[0])
            self.btk.arg("-w")
            self.btk.arg("--bech32")
            self.btk.arg("--p2pkh")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            address_json2 = json.loads(out_text)
            self.assertTrue(address_json2[0])
            self.assertTrue(address_json2[1])

            # match
            self.assertTrue(address_json1[0] == address_json2[0])
            self.assertTrue(address_json1[1] == address_json2[1])

            print("\b", end='')

        sys.stdout.flush()