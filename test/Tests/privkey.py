import sys
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
    },
    {
        "wif": "KyqWNV7VzbrzsyhuDQAzo2qPv3YuiJk5Rp9QwRS2m3ThdMudb5fs",
        "wif_u": "5JQgHCMWE3v1syM9CsZdCnjEMB6RBGMBX6F99Fbfv2TtoJgSb4p",
        "wif_test": "cQCVqQ7MRfZG3RBAboz8AMLTYGrKNkqmVrHt3qtYGA7ht71xfmTK",
        "wif_test_u": "92BJrwB3pGz9r2rRqDTY5PHBzqT8LRtNs376DsxBFmCwaNoc4Px",
    },
    {
        "wif": "KxKDsiQBjoyq84AnzKf82Xk5GSPzeHbJB489AZcDnQzGfr3mNfUn",
        "wif_u": "5J4gAgHahWgA6DxEbHTmBmaKe2gCFLcs7MeR45DWda5NDwFxuSg",
        "wif_test": "cNgDLdQ3Asg6HVe4NjUFPrF8tfhQJjgzF6GcGz4jHXeGvb6nmDZx",
        "wif_test_u": "91qJkR78HjkJ4HTXDdMg4N8HHh2uQWA4TJWN8ha1yJpQzv4P3vk",
    },
    {
        "wif": "Kz2PvtUtw52z7yLxpXUNftHU6mzBg1VTd5cAGSX7xyVmy1xWRuyK",
        "wif_u": "5JT9NGk2LxHmD15XKUJ5TUw2cHzMzmyHuafn9Uz81uDs4gnbFAG",
        "wif_test": "cQPPPoUkN8jFHQpECwHW3CnXj1HbLTb9h7kdNrydU69nDm2edv2V",
        "wif_test_u": "92Dmx1ZZwBMuB4aowpBzL5UzFxM59wWVFXXjE7LdMdxuqh8bDXF",
    },
    {
        "wif": "L2VtEHPMEp9nLLpwWHRHX19xi1N1inMna231WqL1noXLdpvNSVeo",
        "wif_u": "5K1etVdnNFm8Z2KZjtJHRvWDd2Sc4874ysXpF1XbAUJkRDthjy8",
        "wif_test": "cSrshCPCfsr3VnJCthEQtKf2LEfRPETUe4BUdFnXHvBLta5Fuk7K",
        "wif_test_u": "92nHUETKxUqGX5prNECCJX4BGgoKDHeGKpPmKdt6WD3oCG7qmvT",
    },
    {
        "wif": "L4EZ2ismHBC8KtCDbdQwsNiYRNkHiPpRaDtDzv3hodDWsDpPfLpV",
        "wif_u": "5KQTjz4oHMSBGcALcwUBAbpvnrLKjukNRkFKoer6cwRaxMUYaxc",
        "wif_test": "cUbYVdsciEtPVKfUz3E5EhDc3c3hNqv7eG2h7LWDJjsX7xxu5J3y",
        "wif_test_u": "93B6KitLsaWKEffdFHN63CNtSWh2u5HZmh7GtHCbxgAdjUHMFuD",
    },
    {
        "wif": "L1Kc4zFKB5bqghB2SoufWA18ct9BNqMAE1XqBbfzs9dKz7Gxi5JX",
        "wif_u": "5JkBfWjfZC1eK4DEvRg7upohhJUgWR6UTxraaDPRth1ys6aewwT",
        "wif_test": "cRgbXuFAc9J6r8eHqDinsUWCF7Sb3HSrJ3gJJ28WNGHLErMczhta",
        "wif_test_u": "92WpFFZD9R5nH7iXYma2nRMfLxqPfadfouiXeqjwERm2eB3MWAc",
    },
    {
        "wif": "L2u8RxPt7EnzMLgnK5UrXVyUh8JnCVLuGJeVN83uGxCSBHKfqEaP",
        "wif_u": "5K6vLheMeDBsby7kLxPd9GdMmtVx6mwMT8ArUScYnJVRHT6Xirh",
        "wif_test": "cTG7tsPjYJVFWnA3hVHytpUYKMcBrwSbLLnxUYWQn4rSS2NZffk3",
        "wif_test_u": "92sYvSTuESG1a2d2yJHY1sBKRYrfFwUYo52oZ4y483EU4UB8uw4",
    },
    {
        "wif": "L4ZkzcBhWuEhkJQ5xcfiBzZ6einm27bKFtiC5aBjvHTQMmkbxcse",
        "wif_u": "5KUp8ALBzXQwzqoTxCFfLFrQwK5UeQhQAid4sRjmAMVe1ivq6pR",
        "wif_test": "cUvkTXBYwxvxujsMM2UqZK4AGx6AgZh1KvrfBzeFRQ7QcWqirMJc",
        "wif_test_u": "93FShu9jakV5xuJkaY9aCrQNaySBoaEbWfV1x46GW6Egnkh2WRp",
    },
    {
        "wif": "KzZpAtMLUCMXdsGUa9ZXXGN655H7fa9ddrPv5AfiBuNqN7yY8K2g",
        "wif_u": "5JaGDfNNdXDMY1WnfmzDMT8x56W2WanuhyWD2Q2yTVBPHbDjWQ1",
        "wif_test": "cQvodoMBuG3noJjjxZNetas9hJaXL2FKhtYPBb8Dh22qcsAcacYk",
        "wif_test_u": "92LtoQBvDkHVW525J7t8E3guikrjfkL73vNA72PUoDvS4gyPGnS",
    },
]

class Privkey(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Privkey)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("privkey")

    def io_test(self, opts, input, output, input_json=True, input_arg=False, output_json=True):

        for input_group in inputs:
            self.btk.reset()

            for opt in opts:
                self.btk.arg(opt)

            if (input):
                if (input in input_group):
                    if (input_arg):
                        self.btk.arg(input_group[input])
                    elif (input_json):
                        self.btk.set_input(f"[\n\"{input_group[input]}\"\n]")
                    else:
                        self.btk.set_input(input_group[input])
                else:
                    continue

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
                if (output in input_group):
                    self.assertTrue(out_text == input_group[output])

    ###########
    ## Create
    ###########

    def test_0010(self):
        self.io_test(opts=["--create"], input=None, output=None)

    ################
    ## Output Format
    ################

    def test_0020(self):
        self.io_test(opts=["--create", "-J"], input=None, output=None)

    def test_0030(self):
        self.io_test(opts=["--create", "--out-format=json"], input=None, output=None)

    def test_0040(self):
        self.io_test(opts=["--create", "-L"], input=None, output=None, output_json=False)

    def test_0050(self):
        self.io_test(opts=["--create", "--out-format=list"], input=None, output=None, output_json=False)

    ################
    ## Input Format
    ################

    def test_0055(self):
        self.io_test(opts=[], input="string", output="wif", input_arg=True)

    def test_0060(self):
        self.io_test(opts=["-l"], input="string", output="wif", input_json=False)

    def test_0070(self):
        self.io_test(opts=["--in-format=list"], input="string", output="wif", input_json=False)

    def test_0080(self):
        self.io_test(opts=["-j"], input="string", output="wif")

    def test_0090(self):
        self.io_test(opts=["--in-format=json"], input="string", output="wif")


    ###########
    ## String
    ###########

    def test_0100(self):
        self.io_test(opts=[], input="string", output="wif")

    def test_0110(self):
        self.io_test(opts=["-s"], input="string", output="wif")

    def test_0120(self):
        self.io_test(opts=["--in-type=string"], input="string", output="wif")

    def test_0130(self):
        self.io_test(opts=["-s", "-W"], input="string", output="wif")

    def test_0140(self):
        self.io_test(opts=["-s", "--out-type=wif"], input="string", output="wif")

    def test_0150(self):
        self.io_test(opts=["-s", "-X"], input="string", output="hex")

    def test_0160(self):
        self.io_test(opts=["-s", "--out-type=hex"], input="string", output="hex")

    def test_0170(self):
        self.io_test(opts=["-s", "-D"], input="string", output="dec")

    def test_0180(self):
        self.io_test(opts=["-s", "--out-type=decimal"], input="string", output="dec")

    def test_0190(self):
        self.io_test(opts=["-s", "-W", "-C"], input="string", output="wif")

    def test_0200(self):
        self.io_test(opts=["-s", "-W", "-U"], input="string", output="wif_u")

    def test_0210(self):
        self.io_test(opts=["-s", "-W", "--compressed=true"], input="string", output="wif")

    def test_0220(self):
        self.io_test(opts=["-s", "-W", "--compressed=false"], input="string", output="wif_u")

    def test_0230(self):
        self.io_test(opts=["-s", "-X", "-C"], input="string", output="hex_c")

    def test_0240(self):
        self.io_test(opts=["-s", "-X", "-U"], input="string", output="hex_u")

    def test_0250(self):
        self.io_test(opts=["-s", "-X", "--compressed=true"], input="string", output="hex_c")

    def test_0260(self):
        self.io_test(opts=["-s", "-X", "--compressed=false"], input="string", output="hex_u")

    def test_0270(self):
        self.io_test(opts=["-s", "--test"], input="string", output="wif_test")

    def test_0280(self):
        self.io_test(opts=["-s", "--test", "-C"], input="string", output="wif_test")

    def test_0290(self):
        self.io_test(opts=["-s", "--test", "-U"], input="string", output="wif_test_u")

    def test_0300(self):
        self.io_test(opts=["-s", "--test", "--compressed=true"], input="string", output="wif_test")

    def test_0310(self):
        self.io_test(opts=["-s", "--test", "--compressed=false"], input="string", output="wif_test_u")

    def test_0320(self):
        self.io_test(opts=["-s", "-W", "--rehash=5"], input="string", output="wif_rehash5")

    ###########
    ## WIF
    ###########

    def test_0330(self):
        self.io_test(opts=[], input="wif", output="wif")

    def test_0340(self):
        self.io_test(opts=["-w"], input="wif", output="wif")

    def test_0350(self):
        self.io_test(opts=["--in-type=wif"], input="wif", output="wif")

    def test_0360(self):
        self.io_test(opts=["-w", "-W"], input="wif", output="wif")

    def test_0370(self):
        self.io_test(opts=["-w", "--out-type=wif"], input="wif", output="wif")

    def test_0380(self):
        self.io_test(opts=["-w", "-X"], input="wif", output="hex")

    def test_0390(self):
        self.io_test(opts=["-w", "--out-type=hex"], input="wif", output="hex")

    def test_0400(self):
        self.io_test(opts=["-w", "-D"], input="wif", output="dec")

    def test_0410(self):
        self.io_test(opts=["-w", "--out-type=decimal"], input="wif", output="dec")

    def test_0420(self):
        self.io_test(opts=["-w", "-W", "-C"], input="wif", output="wif")

    def test_0430(self):
        self.io_test(opts=["-w", "-W", "-U"], input="wif", output="wif_u")

    def test_0440(self):
        self.io_test(opts=["-w", "-W", "--compressed=true"], input="wif", output="wif")

    def test_0450(self):
        self.io_test(opts=["-w", "-W", "--compressed=false"], input="wif", output="wif_u")

    def test_0460(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif", output="hex_c")

    def test_0470(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif", output="hex_u")

    def test_0480(self):
        self.io_test(opts=["-w", "-X", "--compressed=true"], input="wif", output="hex_c")

    def test_0490(self):
        self.io_test(opts=["-w", "-X", "--compressed=false"], input="wif", output="hex_u")

    def test_0500(self):
        self.io_test(opts=["-w", "--test"], input="wif", output="wif_test")

    def test_0510(self):
        self.io_test(opts=["-w", "--test", "-C"], input="wif", output="wif_test")

    def test_0520(self):
        self.io_test(opts=["-w", "--test", "-U"], input="wif", output="wif_test_u")

    def test_0530(self):
        self.io_test(opts=["-w", "--test", "--compressed=true"], input="wif", output="wif_test")

    def test_0540(self):
        self.io_test(opts=["-w", "--test", "--compressed=false"], input="wif", output="wif_test_u")

    def test_0550(self):
        self.io_test(opts=["-w", "-W", "--rehash=5"], input="wif", output="wif_rehash5")

    ####################
    ## WIF Uncompressed
    ####################

    def test_0560(self):
        self.io_test(opts=[], input="wif_u", output="wif_u")

    def test_0570(self):
        self.io_test(opts=["-w"], input="wif_u", output="wif_u")

    def test_0580(self):
        self.io_test(opts=["--in-type=wif"], input="wif_u", output="wif_u")

    def test_0590(self):
        self.io_test(opts=["-w", "-W"], input="wif_u", output="wif_u")

    def test_0600(self):
        self.io_test(opts=["-w", "--out-type=wif"], input="wif_u", output="wif_u")

    def test_0610(self):
        self.io_test(opts=["-w", "-X"], input="wif_u", output="hex")

    def test_0620(self):
        self.io_test(opts=["-w", "--out-type=hex"], input="wif_u", output="hex")

    def test_0630(self):
        self.io_test(opts=["-w", "-D"], input="wif_u", output="dec")

    def test_0640(self):
        self.io_test(opts=["-w", "--out-type=decimal"], input="wif_u", output="dec")

    def test_0650(self):
        self.io_test(opts=["-w", "-W", "-C"], input="wif_u", output="wif")

    def test_0660(self):
        self.io_test(opts=["-w", "-W", "-U"], input="wif_u", output="wif_u")

    def test_0670(self):
        self.io_test(opts=["-w", "-W", "--compressed=true"], input="wif_u", output="wif")

    def test_0680(self):
        self.io_test(opts=["-w", "-W", "--compressed=false"], input="wif_u", output="wif_u")

    def test_0690(self):
        self.io_test(opts=["-w", "-X", "-C"], input="wif_u", output="hex_c")

    def test_0700(self):
        self.io_test(opts=["-w", "-X", "-U"], input="wif_u", output="hex_u")

    def test_0710(self):
        self.io_test(opts=["-w", "-X", "--compressed=true"], input="wif_u", output="hex_c")

    def test_0720(self):
        self.io_test(opts=["-w", "-X", "--compressed=false"], input="wif_u", output="hex_u")

    def test_0730(self):
        self.io_test(opts=["-w", "--test"], input="wif_u", output="wif_test_u")

    def test_0740(self):
        self.io_test(opts=["-w", "--test", "-C"], input="wif_u", output="wif_test")

    def test_0750(self):
        self.io_test(opts=["-w", "--test", "-U"], input="wif_u", output="wif_test_u")

    def test_0760(self):
        self.io_test(opts=["-w", "--test", "--compressed=true"], input="wif_u", output="wif_test")

    def test_0770(self):
        self.io_test(opts=["-w", "--test", "--compressed=false"], input="wif_u", output="wif_test_u")

    ####################
    ## Hex
    ####################

    def test_0780(self):
        self.io_test(opts=[], input="hex", output="wif")

    def test_0790(self):
        self.io_test(opts=["-x"], input="hex", output="wif")

    def test_0800(self):
        self.io_test(opts=["--in-type=hex"], input="hex", output="wif")

    def test_0810(self):
        self.io_test(opts=["-x", "-W"], input="hex", output="wif")

    def test_0820(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex", output="wif")

    def test_0830(self):
        self.io_test(opts=["-x", "-X"], input="hex", output="hex")

    def test_0840(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex", output="hex")

    def test_0850(self):
        self.io_test(opts=["-x", "-D"], input="hex", output="dec")

    def test_0860(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex", output="dec")

    def test_0870(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex", output="wif")

    def test_0880(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex", output="wif_u")

    def test_0890(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex", output="wif")

    def test_0900(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex", output="wif_u")

    def test_0910(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex", output="hex_c")

    def test_0920(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex", output="hex_u")

    def test_0930(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex", output="hex_c")

    def test_0940(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex", output="hex_u")

    def test_0950(self):
        self.io_test(opts=["-x", "--test"], input="hex", output="wif_test")

    def test_0960(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex", output="wif_test")

    def test_0970(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex", output="wif_test_u")

    def test_0980(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex", output="wif_test")

    def test_0990(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex", output="wif_test_u")

    def test_1000(self):
        self.io_test(opts=["-x", "-W", "--rehash=5"], input="hex", output="wif_rehash5")

    ####################
    ## Hex Compressed
    ####################

    def test_1010(self):
        self.io_test(opts=[], input="hex_c", output="wif")

    def test_1020(self):
        self.io_test(opts=["-x"], input="hex_c", output="wif")

    def test_1030(self):
        self.io_test(opts=["--in-type=hex"], input="hex_c", output="wif")

    def test_1040(self):
        self.io_test(opts=["-x", "-W"], input="hex_c", output="wif")

    def test_1050(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex_c", output="wif")

    def test_1060(self):
        self.io_test(opts=["-x", "-X"], input="hex_c", output="hex")

    def test_1070(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex_c", output="hex")

    def test_1080(self):
        self.io_test(opts=["-x", "-D"], input="hex_c", output="dec")

    def test_1090(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex_c", output="dec")

    def test_1100(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex_c", output="wif")

    def test_1110(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex_c", output="wif_u")

    def test_1120(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex_c", output="wif")

    def test_1130(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex_c", output="wif_u")

    def test_1140(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_c", output="hex_c")

    def test_1150(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_c", output="hex_u")

    def test_1160(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex_c", output="hex_c")

    def test_1170(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex_c", output="hex_u")

    def test_1180(self):
        self.io_test(opts=["-x", "--test"], input="hex_c", output="wif_test")

    def test_1190(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex_c", output="wif_test")

    def test_1200(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex_c", output="wif_test_u")

    def test_1210(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex_c", output="wif_test")

    def test_1220(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex_c", output="wif_test_u")

    def test_1230(self):
        self.io_test(opts=["-x", "-W", "--rehash=5"], input="hex_c", output="wif_rehash5")

    ####################
    ## Hex Uncompressed
    ####################

    def test_1240(self):
        self.io_test(opts=[], input="hex_u", output="wif_u")

    def test_1250(self):
        self.io_test(opts=["-x"], input="hex_u", output="wif_u")

    def test_1260(self):
        self.io_test(opts=["--in-type=hex"], input="hex_u", output="wif_u")

    def test_1270(self):
        self.io_test(opts=["-x", "-W"], input="hex_u", output="wif_u")

    def test_1280(self):
        self.io_test(opts=["-x", "--out-type=wif"], input="hex_u", output="wif_u")

    def test_1290(self):
        self.io_test(opts=["-x", "-X"], input="hex_u", output="hex")

    def test_1300(self):
        self.io_test(opts=["-x", "--out-type=hex"], input="hex_u", output="hex")

    def test_1310(self):
        self.io_test(opts=["-x", "-D"], input="hex_u", output="dec")

    def test_1320(self):
        self.io_test(opts=["-x", "--out-type=decimal"], input="hex_u", output="dec")

    def test_1330(self):
        self.io_test(opts=["-x", "-W", "-C"], input="hex_u", output="wif")

    def test_1340(self):
        self.io_test(opts=["-x", "-W", "-U"], input="hex_u", output="wif_u")

    def test_1350(self):
        self.io_test(opts=["-x", "-W", "--compressed=true"], input="hex_u", output="wif")

    def test_1360(self):
        self.io_test(opts=["-x", "-W", "--compressed=false"], input="hex_u", output="wif_u")

    def test_1370(self):
        self.io_test(opts=["-x", "-X", "-C"], input="hex_u", output="hex_c")

    def test_1380(self):
        self.io_test(opts=["-x", "-X", "-U"], input="hex_u", output="hex_u")

    def test_1390(self):
        self.io_test(opts=["-x", "-X", "--compressed=true"], input="hex_u", output="hex_c")

    def test_1400(self):
        self.io_test(opts=["-x", "-X", "--compressed=false"], input="hex_u", output="hex_u")

    def test_1410(self):
        self.io_test(opts=["-x", "--test"], input="hex_u", output="wif_test_u")

    def test_1420(self):
        self.io_test(opts=["-x", "--test", "-C"], input="hex_u", output="wif_test")

    def test_1430(self):
        self.io_test(opts=["-x", "--test", "-U"], input="hex_u", output="wif_test_u")

    def test_1440(self):
        self.io_test(opts=["-x", "--test", "--compressed=true"], input="hex_u", output="wif_test")

    def test_1450(self):
        self.io_test(opts=["-x", "--test", "--compressed=false"], input="hex_u", output="wif_test_u")

    ####################
    ## Dec
    ####################

    def test_1460(self):
        self.io_test(opts=[], input="dec", output="wif")

    def test_1470(self):
        self.io_test(opts=["-d"], input="dec", output="wif")

    def test_1480(self):
        self.io_test(opts=["--in-type=decimal"], input="dec", output="wif")

    def test_1490(self):
        self.io_test(opts=["-W"], input="dec", output="wif")

    def test_1500(self):
        self.io_test(opts=["--out-type=wif"], input="dec", output="wif")

    def test_1510(self):
        self.io_test(opts=["-X"], input="dec", output="hex")

    def test_1520(self):
        self.io_test(opts=["--out-type=hex"], input="dec", output="hex")

    def test_1530(self):
        self.io_test(opts=["-D"], input="dec", output="dec")

    def test_1540(self):
        self.io_test(opts=["--out-type=decimal"], input="dec", output="dec")

    def test_1550(self):
        self.io_test(opts=["-W", "-C"], input="dec", output="wif")

    def test_1560(self):
        self.io_test(opts=["-W", "-U"], input="dec", output="wif_u")

    def test_1570(self):
        self.io_test(opts=["-W", "--compressed=true"], input="dec", output="wif")

    def test_1580(self):
        self.io_test(opts=["-W", "--compressed=false"], input="dec", output="wif_u")

    def test_1590(self):
        self.io_test(opts=["-X", "-C"], input="dec", output="hex_c")

    def test_1600(self):
        self.io_test(opts=["-X", "-U"], input="dec", output="hex_u")

    def test_1610(self):
        self.io_test(opts=["-X", "--compressed=true"], input="dec", output="hex_c")

    def test_1620(self):
        self.io_test(opts=["-X", "--compressed=false"], input="dec", output="hex_u")

    def test_1630(self):
        self.io_test(opts=["--test"], input="dec", output="wif_test")

    def test_1640(self):
        self.io_test(opts=["--test", "-C"], input="dec", output="wif_test")

    def test_1650(self):
        self.io_test(opts=["--test", "-U"], input="dec", output="wif_test_u")

    def test_1660(self):
        self.io_test(opts=["--test", "--compressed=true"], input="dec", output="wif_test")

    def test_1670(self):
        self.io_test(opts=["--test", "--compressed=false"], input="dec", output="wif_test_u")

    def test_1680(self):
        self.io_test(opts=["-W", "--rehash=5"], input="dec", output="wif_rehash5")

    ####################
    ## Grep
    ####################

    def test_1690(self):

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

    def test_1700(self):

        input = inputs[0]["string"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-s")
        self.btk.arg("-W")
        self.btk.arg("--grep=\"00000000\"")

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertFalse(out.stdout)

    ###############
    ## Match Tests
    ###############

    def test_1710(self):

        progress_char = ["-", "\\", "|", "/"]

        for i in range(100):

            print(progress_char[i % 4], end='')
            sys.stdout.flush()

            self.btk.reset()
            self.btk.arg("--create")
            self.btk.arg("-W")
            self.btk.arg("-X")
            self.btk.arg("-D")
            out = self.btk.run()

            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)

            out_text = out.stdout

            self.assertTrue(out_text)

            create_json = json.loads(out_text)

            self.assertTrue(create_json[0])
            self.assertTrue(create_json[1])
            self.assertTrue(create_json[2])

            ## wif
            self.btk.reset()
            self.btk.set_input(create_json[0])
            self.btk.arg("-w")
            self.btk.arg("-W")
            self.btk.arg("-X")
            self.btk.arg("-D")

            out = self.btk.run()

            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)

            out_text = out.stdout

            self.assertTrue(out_text)

            match_json = json.loads(out_text)

            self.assertTrue(match_json[0])
            self.assertTrue(match_json[1])
            self.assertTrue(match_json[2])

            self.assertTrue(create_json[0] == match_json[0])
            self.assertTrue(create_json[1] == match_json[1])
            self.assertTrue(create_json[2] == match_json[2])

            print("\b", end='')

        sys.stdout.flush()


