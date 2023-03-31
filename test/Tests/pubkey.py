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

    def test_output_format_0010(self):
        self.io_test(opts=["-w", "-J"], input="wif", output=None)

    def test_output_format_0020(self):
        self.io_test(opts=["-w", "--out-format=json"], input="wif", output=None)

    def test_output_format_0030(self):
        self.io_test(opts=["-w", "-L"], input="wif", output=None, output_json=False)

    def test_output_format_0040(self):
        self.io_test(opts=["-w", "--out-format=list"], input="wif", output=None, output_json=False)

    ################
    ## Input Format
    ################

    def test_input_format_0010(self):
        self.io_test(opts=["-w", "-l"], input="wif", output=None, input_json=False)

    def test_input_format_0020(self):
        self.io_test(opts=["-w", "--in-format=list"], input="wif", output=None, input_json=False)

    def test_input_format_0030(self):
        self.io_test(opts=["-w", "-j"], input="wif", output=None)

    def test_input_format_0040(self):
        self.io_test(opts=["-w", "--in-format=json"], input="wif", output=None)

    ###########
    ## WIF
    ###########

    def test_wif_0010(self):
        self.io_test(opts=[], input="wif", output="hex")

    def test_wif_0020(self):
        self.io_test(opts=["-w"], input="wif", output="hex")

    def test_wif_0030(self):
        self.io_test(opts=["--in-type=wif"], input="wif", output="hex")

    def test_wif_0040(self):
        self.io_test(opts=["-w", "-C"], input="wif", output="hex")

    def test_wif_0050(self):
        self.io_test(opts=["-w", "-U"], input="wif", output="hex_u")

    def test_wif_0060(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif", output="hex")

    def test_wif_0070(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif", output="hex_u")

    def test_wif_0080(self):
        self.io_test(opts=["-w", "-X"], input="wif", output="hex")

    def test_wif_0090(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif", output="hex")

    def test_wif_0100(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif", output="hex_u")

    ###################
    ## WIF Uncompressed
    ###################

    def test_wifu_0010(self):
        self.io_test(opts=[], input="wif_u", output="hex_u")

    def test_wifu_0020(self):
        self.io_test(opts=["-w"], input="wif_u", output="hex_u")

    def test_wifu_0030(self):
        self.io_test(opts=["--in-type=wif"], input="wif_u", output="hex_u")

    def test_wifu_0040(self):
        self.io_test(opts=["-w", "-C"], input="wif_u", output="hex")

    def test_wifu_0050(self):
        self.io_test(opts=["-w", "-U"], input="wif_u", output="hex_u")

    def test_wifu_0060(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_u", output="hex")

    def test_wifu_0070(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_u", output="hex_u")

    def test_wifu_0080(self):
        self.io_test(opts=["-w", "-X"], input="wif_u", output="hex_u")

    def test_wifu_0090(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_u", output="hex")

    def test_wifu_0100(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_u", output="hex_u")

    ###################
    ## WIF Test
    ###################

    def test_wiftest_0010(self):
        self.io_test(opts=[], input="wif_test", output="hex")

    def test_wiftest_0020(self):
        self.io_test(opts=["-w"], input="wif_test", output="hex")

    def test_wiftest_0030(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test", output="hex")

    def test_wiftest_0040(self):
        self.io_test(opts=["-w", "-C"], input="wif_test", output="hex")

    def test_wiftest_0050(self):
        self.io_test(opts=["-w", "-U"], input="wif_test", output="hex_u")

    def test_wiftest_0060(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_test", output="hex")

    def test_wiftest_0070(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_test", output="hex_u")

    def test_wiftest_0080(self):
        self.io_test(opts=["-w", "-X"], input="wif_test", output="hex")

    def test_wiftest_0090(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_test", output="hex")

    def test_wiftest_0100(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_test", output="hex_u")

    #########################
    ## WIF Test Uncompressed
    #########################

    def test_wiftestu_0010(self):
        self.io_test(opts=[], input="wif_test_u", output="hex_u")

    def test_wiftestu_0020(self):
        self.io_test(opts=["-w"], input="wif_test_u", output="hex_u")

    def test_wiftestu_0030(self):
        self.io_test(opts=["--in-type=wif"], input="wif_test_u", output="hex_u")

    def test_wiftestu_0040(self):
        self.io_test(opts=["-w", "-C"], input="wif_test_u", output="hex")

    def test_wiftestu_0050(self):
        self.io_test(opts=["-w", "-U"], input="wif_test_u", output="hex_u")

    def test_wiftestu_0060(self):
        self.io_test(opts=["-w", "--compressed=true"], input="wif_test_u", output="hex")

    def test_wiftestu_0070(self):
        self.io_test(opts=["-w", "--compressed=false"], input="wif_test_u", output="hex_u")

    def test_wiftestu_0080(self):
        self.io_test(opts=["-w", "-X"], input="wif_test_u", output="hex_u")

    def test_wiftestu_0090(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_test_u", output="hex")

    def test_wiftestu_0100(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_test_u", output="hex_u")

    #######
    ## Hex
    #######

    def test_hex_0010(self):
        self.io_test(opts=[], input="hex", output="hex")

    def test_hex_0020(self):
        self.io_test(opts=["-x"], input="hex", output="hex")

    def test_hex_0030(self):
        self.io_test(opts=["--in-type=hex"], input="hex", output="hex")

    def test_hex_0040(self):
        self.io_test(opts=["-x", "-C"], input="hex", output="hex")

    def test_hex_0050(self):
        self.io_test(opts=["-x", "-U"], input="hex", output="hex_u")

    def test_hex_0060(self):
        self.io_test(opts=["-x", "--compressed=true"], input="hex", output="hex")

    def test_hex_0070(self):
        self.io_test(opts=["-x", "--compressed=false"], input="hex", output="hex_u")

    def test_hex_0080(self):
        self.io_test(opts=["-x", "-X"], input="hex", output="hex")

    def test_hex_0090(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex", output="hex")

    def test_hex_0100(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex", output="hex_u")

    ####################
    ## Hex Uncompressed
    ####################

    def test_hexu_0010(self):
        self.io_test(opts=[], input="hex_u", output="hex_u")

    def test_hexu_0020(self):
        self.io_test(opts=["-x"], input="hex_u", output="hex_u")

    def test_hexu_0030(self):
        self.io_test(opts=["--in-type=hex"], input="hex_u", output="hex_u")

    def test_hexu_0040(self):
        self.io_test(opts=["-x", "-C"], input="hex_u", output="hex")

    def test_hexu_0050(self):
        self.io_test(opts=["-x", "-U"], input="hex_u", output="hex_u")

    def test_hexu_0060(self):
        self.io_test(opts=["-x", "--compressed=true"], input="hex_u", output="hex")

    def test_hexu_0070(self):
        self.io_test(opts=["-x", "--compressed=false"], input="hex_u", output="hex_u")

    def test_hexu_0080(self):
        self.io_test(opts=["-x", "-X"], input="hex_u", output="hex_u")

    def test_hexu_0090(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_u", output="hex")

    def test_hexu_0100(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_u", output="hex_u")

    ####################
    ## Grep
    ####################

    def test_grep_exists(self):

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

    def test_grep_noexists(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("-X")
        self.btk.arg("--grep=\"00000000\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertFalse(out.stdout)