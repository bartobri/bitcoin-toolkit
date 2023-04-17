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
    }
]

class Pubkey(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Pubkey)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("pubkey")

    def io_test(self, opts, input, output, input_json=True, input_arg=False, output_json=True):

        for input_group in inputs:
            self.btk.reset()

            for opt in opts:
                self.btk.arg(opt)

            if (input):
                if (input_arg):
                    self.btk.arg(input_group[input])
                elif (input_json):
                    self.btk.set_input(f"[\n\"{input_group[input]}\"\n]")
                else:
                    self.btk.set_input(input_group[input])

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

    def test_0045(self):
        self.io_test(opts=["-w"], input="wif", output="hex", input_arg=True)

    def test_0050(self):
        self.io_test(opts=["-w", "-l"], input="wif", output="hex", input_json=False)

    def test_0060(self):
        self.io_test(opts=["-w", "--in-format=list"], input="wif", output="hex", input_json=False)

    def test_0070(self):
        self.io_test(opts=["-w", "-j"], input="wif", output="hex")

    def test_0080(self):
        self.io_test(opts=["-w", "--in-format=json"], input="wif", output="hex")

    ###########
    ## WIF
    ###########

    def test_0090(self):
        self.io_test(opts=[], input="wif", output="hex")

    def test_0100(self):
        self.io_test(opts=["-w"], input="wif", output="hex")

    def test_0110(self):
        self.io_test(opts=["--in-type=wif"], input="wif", output="hex")

    def test_0120(self):
        self.io_test(opts=["-w", "-C"], input="wif", output="hex")

    def test_0130(self):
        self.io_test(opts=["-w", "-U"], input="wif", output="hex_u")

    def test_0140(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif", output="hex")

    def test_0150(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif", output="hex_u")

    def test_0160(self):
        self.io_test(opts=["-w", "-X"], input="wif", output="hex")

    def test_0170(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif", output="hex")

    def test_0180(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif", output="hex_u")

    ###################
    ## WIF Uncompressed
    ###################

    def test_0185(self):
        self.io_test(opts=[], input="wif_u", output="hex_u")

    def test_0190(self):
        self.io_test(opts=["-w"], input="wif_u", output="hex_u")

    def test_0200(self):
        self.io_test(opts=["--in-type=wif"], input="wif_u", output="hex_u")

    def test_0210(self):
        self.io_test(opts=["-w", "-C"], input="wif_u", output="hex")

    def test_0220(self):
        self.io_test(opts=["-w", "-U"], input="wif_u", output="hex_u")

    def test_0230(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_u", output="hex")

    def test_0240(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_u", output="hex_u")

    def test_0250(self):
        self.io_test(opts=["-w", "-X"], input="wif_u", output="hex_u")

    def test_0260(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_u", output="hex")

    def test_0270(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_u", output="hex_u")

    ###################
    ## WIF Test
    ###################

    def test_0280(self):
        self.io_test(opts=[], input="wif_test", output="hex")

    def test_0290(self):
        self.io_test(opts=["-w"], input="wif_test", output="hex")

    def test_0300(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test", output="hex")

    def test_0310(self):
        self.io_test(opts=["-w", "-C"], input="wif_test", output="hex")

    def test_0320(self):
        self.io_test(opts=["-w", "-U"], input="wif_test", output="hex_u")

    def test_0330(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_test", output="hex")

    def test_0340(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_test", output="hex_u")

    def test_0350(self):
        self.io_test(opts=["-w", "-X"], input="wif_test", output="hex")

    def test_0360(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_test", output="hex")

    def test_0370(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_test", output="hex_u")

    #########################
    ## WIF Test Uncompressed
    #########################

    def test_0380(self):
        self.io_test(opts=[], input="wif_test_u", output="hex_u")

    def test_0390(self):
        self.io_test(opts=["-w"], input="wif_test_u", output="hex_u")

    def test_0400(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test_u", output="hex_u")

    def test_0410(self):
        self.io_test(opts=["-w", "-C"], input="wif_test_u", output="hex")

    def test_0420(self):
        self.io_test(opts=["-w", "-U"], input="wif_test_u", output="hex_u")

    def test_0430(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_test_u", output="hex")

    def test_0440(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_test_u", output="hex_u")

    def test_0450(self):
        self.io_test(opts=["-w", "-X"], input="wif_test_u", output="hex_u")

    def test_0460(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_test_u", output="hex")

    def test_0470(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_test_u", output="hex_u")

    #######
    ## Hex
    #######

    def test_0480(self):
        self.io_test(opts=[], input="hex", output="hex")

    def test_0490(self):
        self.io_test(opts=["-x"], input="hex", output="hex")

    def test_0500(self):
        self.io_test(opts=["--in-type=hex"], input="hex", output="hex")

    def test_0510(self):
        self.io_test(opts=["-x", "-C"], input="hex", output="hex")

    def test_0520(self):
        self.io_test(opts=["-x", "-U"], input="hex", output="hex_u")

    def test_0530(self):
        self.io_test(opts=["-x", "--compressed=true"], input="hex", output="hex")

    def test_0540(self):
        self.io_test(opts=["-x", "--compressed=false"], input="hex", output="hex_u")

    def test_0550(self):
        self.io_test(opts=["-x", "-X"], input="hex", output="hex")

    def test_0560(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex", output="hex")

    def test_0570(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex", output="hex_u")

    ####################
    ## Hex Uncompressed
    ####################

    def test_0580(self):
        self.io_test(opts=[], input="hex_u", output="hex_u")

    def test_0590(self):
        self.io_test(opts=["-x"], input="hex_u", output="hex_u")

    def test_0600(self):
        self.io_test(opts=["--in-type=hex"], input="hex_u", output="hex_u")

    def test_0610(self):
        self.io_test(opts=["-x", "-C"], input="hex_u", output="hex")

    def test_0620(self):
        self.io_test(opts=["-x", "-U"], input="hex_u", output="hex_u")

    def test_0630(self):
        self.io_test(opts=["-x", "--compressed=true"], input="hex_u", output="hex")

    def test_0640(self):
        self.io_test(opts=["-x", "--compressed=false"], input="hex_u", output="hex_u")

    def test_0650(self):
        self.io_test(opts=["-x", "-X"], input="hex_u", output="hex_u")

    def test_0660(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_u", output="hex")

    def test_0670(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_u", output="hex_u")

    ####################
    ## Grep
    ####################

    def test_0680(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("-X")
        self.btk.arg("--grep=\"039927c7\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)

        out_text = out.stdout

        self.assertTrue(out_text)

    def test_0690(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("-X")
        self.btk.arg("--grep=\"00000000\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertFalse(out.stdout)

    ###############
    ## Match Tests
    ###############

    def test_0700(self):

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
            self.btk.reset("pubkey")
            self.btk.set_input(privkey_json[0])
            self.btk.arg("-w")
            self.btk.arg("-X")
            self.btk.arg("-C")
            self.btk.arg("-U")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            pubkey_json1 = json.loads(out_text)
            self.assertTrue(pubkey_json1[0])
            self.assertTrue(pubkey_json1[1])

            # create pubkey
            self.btk.reset()
            self.btk.set_input(privkey_json[0])
            self.btk.arg("-w")
            self.btk.arg("-X")
            self.btk.arg("-C")
            self.btk.arg("-U")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            pubkey_json2 = json.loads(out_text)
            self.assertTrue(pubkey_json2[0])
            self.assertTrue(pubkey_json2[1])

            # match
            self.assertTrue(pubkey_json1[0] == pubkey_json2[0])
            self.assertTrue(pubkey_json1[1] == pubkey_json2[1])

            print("\b", end='')

        sys.stdout.flush()


    def test_0710(self):

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
            self.btk.reset("pubkey")
            self.btk.set_input(privkey_json[0])
            self.btk.arg("-w")
            self.btk.arg("-X")
            self.btk.arg("-C")
            self.btk.arg("-U")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            pubkey_json = json.loads(out_text)
            self.assertTrue(pubkey_json[0])
            self.assertTrue(pubkey_json[1])

            # create pubkey match 1
            self.btk.reset()
            self.btk.set_input(pubkey_json[0])
            self.btk.arg("-x")
            self.btk.arg("-X")
            self.btk.arg("-C")
            self.btk.arg("-U")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            pubkey_match_json = json.loads(out_text)
            self.assertTrue(pubkey_match_json[0])
            self.assertTrue(pubkey_match_json[1])
            self.assertTrue(pubkey_match_json[0] == pubkey_json[0])
            self.assertTrue(pubkey_match_json[1] == pubkey_json[1])

            # create pubkey match 2
            self.btk.reset()
            self.btk.set_input(pubkey_json[1])
            self.btk.arg("-x")
            self.btk.arg("-X")
            self.btk.arg("-C")
            self.btk.arg("-U")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            pubkey_match_json2 = json.loads(out_text)
            self.assertTrue(pubkey_match_json2[0])
            self.assertTrue(pubkey_match_json2[1])
            self.assertTrue(pubkey_match_json2[0] == pubkey_json[0])
            self.assertTrue(pubkey_match_json2[1] == pubkey_json[1])

            print("\b", end='')

        sys.stdout.flush()