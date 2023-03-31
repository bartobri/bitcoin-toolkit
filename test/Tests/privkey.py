import json
import unittest
from .btk import BTK

inputs = [
    {
        "string": "test01",
        "wif": "Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu",
        "wif_u": "5JbtoEnCt6yAWCUvKwKYeCitigTV5qzTHtwHKa7Lhuk4sYDnTpP",
        "wif_test": "cR415zpNsd3A7K9VkzZz2XExuCTJcqLH7cP49PbVUgT6LaJiWb1Q",
        "wif_test_u": "92NXNybkUL3JUFzCxHDTWoGrNLpCF1XedqoEQCTr3eV7ebgGHp5",
        "wif_rehash5": "L3L3nfrhGAYR4ekMEWAwB4sjJ9rNVjFaa4CWdCbsSxbFccCk9bgA",
        "hex": "678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b296",
        "hex_c": "678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b29601",
        "hex_u": "678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b29600",
        "dec": "46840018765432835575846246602607830865213402427381122709315538919381702914710"
    }
]

class Privkey(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Privkey)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("privkey")

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

    ###########
    ## Create
    ###########

    def test_create_0010(self):
        self.io_test(opts=["--create"], input=None, output=None)

    ################
    ## Output Format
    ################

    def test_output_format_0010(self):
        self.io_test(opts=["--create", "-J"], input=None, output=None)

    def test_output_format_0020(self):
        self.io_test(opts=["--create", "--out-format=json"], input=None, output=None)

    def test_output_format_0030(self):
        self.io_test(opts=["--create", "-L"], input=None, output=None, output_json=False)

    def test_output_format_0040(self):
        self.io_test(opts=["--create", "--out-format=list"], input=None, output=None, output_json=False)

    ################
    ## Input Format
    ################

    def test_input_format_0010(self):
        self.io_test(opts=["-l"], input="string", output="wif", input_json=False)

    def test_input_format_0020(self):
        self.io_test(opts=["--in-format=list"], input="string", output="wif", input_json=False)

    def test_input_format_0030(self):
        self.io_test(opts=["-j"], input="string", output="wif")

    def test_input_format_0040(self):
        self.io_test(opts=["--in-format=json"], input="string", output="wif")

    ###########
    ## String
    ###########

    def test_string_0010(self):
        self.io_test(opts=[], input="string", output="wif")

    def test_string_0011(self):
        self.io_test(opts=["-s"], input="string", output="wif")

    def test_string_0012(self):
        self.io_test(opts=["--in-type=string"], input="string", output="wif")

    def test_string_0013(self):
        self.io_test(opts=["-s", "-W"], input="string", output="wif")

    def test_string_0014(self):
        self.io_test(opts=["-s", "--out-type=wif"], input="string", output="wif")

    def test_string_0020(self):
        self.io_test(opts=["-s", "-X"], input="string", output="hex")

    def test_string_0030(self):
        self.io_test(opts=["-s", "--out-type=hex"], input="string", output="hex")

    def test_string_0040(self):
        self.io_test(opts=["-s", "-D"], input="string", output="dec")

    def test_string_0050(self):
        self.io_test(opts=["-s", "--out-type=decimal"], input="string", output="dec")

    def test_string_0060(self):
        self.io_test(opts=["-s", "-W", "-C"], input="string", output="wif")

    def test_string_0061(self):
        self.io_test(opts=["-s", "-W", "-U"], input="string", output="wif_u")

    def test_string_0062(self):
        self.io_test(opts=["-s", "-W", "--compressed=true"], input="string", output="wif")

    def test_string_0063(self):
        self.io_test(opts=["-s", "-W", "--compressed=false"], input="string", output="wif_u")

    def test_string_0070(self):
        self.io_test(opts=["-s", "-X", "-C"], input="string", output="hex_c")

    def test_string_0071(self):
        self.io_test(opts=["-s", "-X", "-U"], input="string", output="hex_u")

    def test_string_0072(self):
        self.io_test(opts=["-s", "-X", "--compressed=true"], input="string", output="hex_c")

    def test_string_0073(self):
        self.io_test(opts=["-s", "-X", "--compressed=false"], input="string", output="hex_u")

    def test_string_0080(self):
        self.io_test(opts=["-s", "--test"], input="string", output="wif_test")

    def test_string_0081(self):
        self.io_test(opts=["-s", "--test", "-C"], input="string", output="wif_test")

    def test_string_0082(self):
        self.io_test(opts=["-s", "--test", "-U"], input="string", output="wif_test_u")

    def test_string_0083(self):
        self.io_test(opts=["-s", "--test", "--compressed=true"], input="string", output="wif_test")

    def test_string_0084(self):
        self.io_test(opts=["-s", "--test", "--compressed=false"], input="string", output="wif_test_u")

    def test_string_0090(self):
        self.io_test(opts=["-s", "-W", "--rehash=5"], input="string", output="wif_rehash5")

    ###########
    ## WIF
    ###########

    def test_wif_0010(self):
        self.io_test(opts=[], input="wif", output="wif")

    def test_wif_0011(self):
        self.io_test(opts=["-w"], input="wif", output="wif")

    def test_wif_0012(self):
        self.io_test(opts=["--in-type=wif"], input="wif", output="wif")

    def test_wif_0013(self):
        self.io_test(opts=["-w", "-W"], input="wif", output="wif")

    def test_wif_0014(self):
        self.io_test(opts=["-w", "--out-type=wif"], input="wif", output="wif")

    def test_wif_0020(self):
        self.io_test(opts=["-w", "-X"], input="wif", output="hex")

    def test_wif_0030(self):
        self.io_test(opts=["-w", "--out-type=hex"], input="wif", output="hex")

    def test_wif_0040(self):
        self.io_test(opts=["-w", "-D"], input="wif", output="dec")

    def test_wif_0050(self):
        self.io_test(opts=["-w", "--out-type=decimal"], input="wif", output="dec")

    def test_wif_0060(self):
        self.io_test(opts=["-w", "-W", "-C"], input="wif", output="wif")

    def test_wif_0061(self):
        self.io_test(opts=["-w", "-W", "-U"], input="wif", output="wif_u")

    def test_wif_0062(self):
        self.io_test(opts=["-w", "-W", "--compressed=true"], input="wif", output="wif")

    def test_wif_0063(self):
        self.io_test(opts=["-w", "-W", "--compressed=false"], input="wif", output="wif_u")

    def test_wif_0070(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif", output="hex_c")

    def test_wif_0071(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif", output="hex_u")

    def test_wif_0072(self):
        self.io_test(opts=["-w", "-X", "--compressed=true"], input="wif", output="hex_c")

    def test_wif_0073(self):
        self.io_test(opts=["-w", "-X", "--compressed=false"], input="wif", output="hex_u")

    def test_wif_0080(self):
        self.io_test(opts=["-w", "--test"], input="wif", output="wif_test")

    def test_wif_0081(self):
        self.io_test(opts=["-w", "--test", "-C"], input="wif", output="wif_test")

    def test_wif_0082(self):
        self.io_test(opts=["-w", "--test", "-U"], input="wif", output="wif_test_u")

    def test_wif_0083(self):
        self.io_test(opts=["-w", "--test", "--compressed=true"], input="wif", output="wif_test")

    def test_wif_0084(self):
        self.io_test(opts=["-w", "--test", "--compressed=false"], input="wif", output="wif_test_u")

    def test_wif_0090(self):
        self.io_test(opts=["-w", "-W", "--rehash=5"], input="wif", output="wif_rehash5")

    ####################
    ## WIF Uncompressed
    ####################

    def test_wifu_0010(self):
        self.io_test(opts=[], input="wif_u", output="wif_u")

    def test_wifu_0011(self):
        self.io_test(opts=["-w"], input="wif_u", output="wif_u")

    def test_wifu_0012(self):
        self.io_test(opts=["--in-type=wif"], input="wif_u", output="wif_u")

    def test_wifu_0013(self):
        self.io_test(opts=["-w", "-W"], input="wif_u", output="wif_u")

    def test_wifu_0014(self):
        self.io_test(opts=["-w", "--out-type=wif"], input="wif_u", output="wif_u")

    def test_wifu_0020(self):
        self.io_test(opts=["-w", "-X"], input="wif_u", output="hex")

    def test_wifu_0030(self):
        self.io_test(opts=["-w", "--out-type=hex"], input="wif_u", output="hex")

    def test_wifu_0040(self):
        self.io_test(opts=["-w", "-D"], input="wif_u", output="dec")

    def test_wifu_0050(self):
        self.io_test(opts=["-w", "--out-type=decimal"], input="wif_u", output="dec")

    def test_wifu_0060(self):
        self.io_test(opts=["-w", "-W", "-C"], input="wif_u", output="wif")

    def test_wifu_0061(self):
        self.io_test(opts=["-w", "-W", "-U"], input="wif_u", output="wif_u")

    def test_wifu_0062(self):
        self.io_test(opts=["-w", "-W", "--compressed=true"], input="wif_u", output="wif")

    def test_wifu_0063(self):
        self.io_test(opts=["-w", "-W", "--compressed=false"], input="wif_u", output="wif_u")

    def test_wifu_0070(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_u", output="hex_c")

    def test_wifu_0071(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_u", output="hex_u")

    def test_wifu_0072(self):
        self.io_test(opts=["-w", "-X", "--compressed=true"], input="wif_u", output="hex_c")

    def test_wifu_0073(self):
        self.io_test(opts=["-w", "-X", "--compressed=false"], input="wif_u", output="hex_u")

    def test_wifu_0080(self):
        self.io_test(opts=["-w", "--test"], input="wif_u", output="wif_test_u")

    def test_wifu_0081(self):
        self.io_test(opts=["-w", "--test", "-C"], input="wif_u", output="wif_test")

    def test_wifu_0082(self):
        self.io_test(opts=["-w", "--test", "-U"], input="wif_u", output="wif_test_u")

    def test_wifu_0083(self):
        self.io_test(opts=["-w", "--test", "--compressed=true"], input="wif_u", output="wif_test")

    def test_wifu_0084(self):
        self.io_test(opts=["-w", "--test", "--compressed=false"], input="wif_u", output="wif_test_u")

    ####################
    ## Hex
    ####################

    def test_hex_0010(self):
        self.io_test(opts=[], input="hex", output="wif")

    def test_hex_0011(self):
        self.io_test(opts=["-x"], input="hex", output="wif")

    def test_hex_0012(self):
        self.io_test(opts=["--in-type=hex"], input="hex", output="wif")

    def test_hex_0013(self):
        self.io_test(opts=["-x", "-W"], input="hex", output="wif")

    def test_hex_0014(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex", output="wif")

    def test_hex_0020(self):
        self.io_test(opts=["-x", "-X"], input="hex", output="hex")

    def test_hex_0030(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex", output="hex")

    def test_hex_0040(self):
        self.io_test(opts=["-x", "-D"], input="hex", output="dec")

    def test_hex_0050(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex", output="dec")

    def test_hex_0060(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex", output="wif")

    def test_hex_0061(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex", output="wif_u")

    def test_hex_0062(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex", output="wif")

    def test_hex_0063(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex", output="wif_u")

    def test_hex_0070(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex", output="hex_c")

    def test_hex_0071(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex", output="hex_u")

    def test_hex_0072(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex", output="hex_c")

    def test_hex_0073(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex", output="hex_u")

    def test_hex_0080(self):
        self.io_test(opts=["-x", "--test"], input="hex", output="wif_test")

    def test_hex_0081(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex", output="wif_test")

    def test_hex_0082(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex", output="wif_test_u")

    def test_hex_0083(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex", output="wif_test")

    def test_hex_0084(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex", output="wif_test_u")

    def test_hex_0090(self):
        self.io_test(opts=["-x", "-W", "--rehash=5"], input="hex", output="wif_rehash5")

    ####################
    ## Hex Compressed
    ####################

    def test_hexc_0010(self):
        self.io_test(opts=[], input="hex_c", output="wif")

    def test_hexc_0011(self):
        self.io_test(opts=["-x"], input="hex_c", output="wif")

    def test_hexc_0012(self):
        self.io_test(opts=["--in-type=hex"], input="hex_c", output="wif")

    def test_hexc_0013(self):
        self.io_test(opts=["-x", "-W"], input="hex_c", output="wif")

    def test_hexc_0014(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex_c", output="wif")

    def test_hexc_0020(self):
        self.io_test(opts=["-x", "-X"], input="hex_c", output="hex")

    def test_hexc_0030(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex_c", output="hex")

    def test_hexc_0040(self):
        self.io_test(opts=["-x", "-D"], input="hex_c", output="dec")

    def test_hexc_0050(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex_c", output="dec")

    def test_hexc_0060(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex_c", output="wif")

    def test_hexc_0061(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex_c", output="wif_u")

    def test_hexc_0062(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex_c", output="wif")

    def test_hexc_0063(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex_c", output="wif_u")

    def test_hexc_0070(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_c", output="hex_c")

    def test_hexc_0071(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_c", output="hex_u")

    def test_hexc_0072(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex_c", output="hex_c")

    def test_hexc_0073(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex_c", output="hex_u")

    def test_hexc_0080(self):
        self.io_test(opts=["-x", "--test"], input="hex_c", output="wif_test")

    def test_hexc_0081(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex_c", output="wif_test")

    def test_hexc_0082(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex_c", output="wif_test_u")

    def test_hexc_0083(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex_c", output="wif_test")

    def test_hexc_0084(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex_c", output="wif_test_u")

    def test_hexc_0090(self):
        self.io_test(opts=["-x", "-W", "--rehash=5"], input="hex_c", output="wif_rehash5")

    ####################
    ## Hex Uncompressed
    ####################

    def test_hexu_0010(self):
        self.io_test(opts=[], input="hex_u", output="wif_u")

    def test_hexu_0011(self):
        self.io_test(opts=["-x"], input="hex_u", output="wif_u")

    def test_hexu_0012(self):
        self.io_test(opts=["--in-type=hex"], input="hex_u", output="wif_u")

    def test_hexu_0013(self):
        self.io_test(opts=["-x", "-W"], input="hex_u", output="wif_u")

    def test_hexu_0014(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex_u", output="wif_u")

    def test_hexu_0020(self):
        self.io_test(opts=["-x", "-X"], input="hex_u", output="hex")

    def test_hexu_0030(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex_u", output="hex")

    def test_hexu_0040(self):
        self.io_test(opts=["-x", "-D"], input="hex_u", output="dec")

    def test_hexu_0050(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex_u", output="dec")

    def test_hexu_0060(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex_u", output="wif")

    def test_hexu_0061(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex_u", output="wif_u")

    def test_hexu_0062(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex_u", output="wif")

    def test_hexu_0063(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex_u", output="wif_u")

    def test_hexu_0070(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_u", output="hex_c")

    def test_hexu_0071(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_u", output="hex_u")

    def test_hexu_0072(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex_u", output="hex_c")

    def test_hexu_0073(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex_u", output="hex_u")

    def test_hexu_0080(self):
        self.io_test(opts=["-x", "--test"], input="hex_u", output="wif_test_u")

    def test_hexu_0081(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex_u", output="wif_test")

    def test_hexu_0082(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex_u", output="wif_test_u")

    def test_hexu_0083(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex_u", output="wif_test")

    def test_hexu_0084(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex_u", output="wif_test_u")

    ####################
    ## Dec
    ####################

    def test_dec_0010(self):
        self.io_test(opts=[], input="dec", output="wif")

    def test_dec_0011(self):
        self.io_test(opts=["-d"], input="dec", output="wif")

    def test_dec_0012(self):
        self.io_test(opts=["--in-type=decimal"], input="dec", output="wif")

    def test_dec_0013(self):
        self.io_test(opts=["-W"], input="dec", output="wif")

    def test_dec_0014(self):
        self.io_test(opts=["--out-type=wif"], input="dec", output="wif")

    def test_dec_0020(self):
        self.io_test(opts=["-X"], input="dec", output="hex")

    def test_dec_0030(self):
        self.io_test(opts=["--out-type=hex"], input="dec", output="hex")

    def test_dec_0040(self):
        self.io_test(opts=["-D"], input="dec", output="dec")

    def test_dec_0050(self):
        self.io_test(opts=["--out-type=decimal"], input="dec", output="dec")

    def test_dec_0060(self):
        self.io_test(opts=["-W", "-C"], input="dec", output="wif")

    def test_dec_0061(self):
        self.io_test(opts=["-W", "-U"], input="dec", output="wif_u")

    def test_dec_0062(self):
        self.io_test(opts=["-W", "--compressed=true"], input="dec", output="wif")

    def test_dec_0063(self):
        self.io_test(opts=["-W", "--compressed=false"], input="dec", output="wif_u")

    def test_dec_0070(self):
        self.io_test(opts=["-X", "-C"], input="dec", output="hex_c")

    def test_dec_0071(self):
        self.io_test(opts=["-X", "-U"], input="dec", output="hex_u")

    def test_dec_0072(self):
        self.io_test(opts=["-X", "--compressed=true"], input="dec", output="hex_c")

    def test_dec_0073(self):
        self.io_test(opts=["-X", "--compressed=false"], input="dec", output="hex_u")

    def test_dec_0080(self):
        self.io_test(opts=["--test"], input="dec", output="wif_test")

    def test_dec_0081(self):
        self.io_test(opts=["--test", "-C"], input="dec", output="wif_test")

    def test_dec_0082(self):
        self.io_test(opts=["--test", "-U"], input="dec", output="wif_test_u")

    def test_dec_0083(self):
        self.io_test(opts=["--test", "--compressed=true"], input="dec", output="wif_test")

    def test_dec_0084(self):
        self.io_test(opts=["--test", "--compressed=false"], input="dec", output="wif_test_u")

    def test_dec_0090(self):
        self.io_test(opts=["-W", "--rehash=5"], input="dec", output="wif_rehash5")

    ####################
    ## Grep
    ####################

    def test_grep_exists(self):

        input = inputs[0]["string"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-s")
        self.btk.arg("-W")
        self.btk.arg("--grep=\"Kzh1d5pX\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)

        out_text = out.stdout

        self.assertTrue(out_text)

    def test_grep_noexists(self):

        input = inputs[0]["string"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-s")
        self.btk.arg("-W")
        self.btk.arg("--grep=\"00000000\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertFalse(out.stdout)



