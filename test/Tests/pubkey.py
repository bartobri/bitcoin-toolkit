import sys
import json
import unittest
from .btk import BTK

inputs = [
    {
        "wif": "KwZyzjLqrxTdMBUXV1pNsF53jmatRNSZn7t5x8sx735rzAbz4Cpa",
        "wif_u": "5HtsyacJsHeJJG4X1ukMpBSPNUKZ3MGJrC5Ko2LFJEAFxC1Quux",
        "wif_test": "cMvyTeLhJ29tWcwnsRdWEZa7MztJ5pYFrA2Z4ZLTc9jsEuiM9R3d",
        "wif_test_u": "91fWZKRrTWiSGKZoeFeGgmzM28gGCWoWC8wGsegkdxuJj9F5Ju6",
        "hex": "03aeed4c495e665e8f81d503edb3972f9605d467e39bdf4e807846ef2b8faf7de6",
        "hex_u": "04aeed4c495e665e8f81d503edb3972f9605d467e39bdf4e807846ef2b8faf7de61b8daa3aa7f4e706d48b902556560376760cd1a65b657e5bc2f5f278b6d62321",
    },
    {
        "wif": "KyqWNV7VzbrzsyhuDQAzo2qPv3YuiJk5Rp9QwRS2m3ThdMudb5fs",
        "wif_u": "5JQgHCMWE3v1syM9CsZdCnjEMB6RBGMBX6F99Fbfv2TtoJgSb4p",
        "wif_test": "cQCVqQ7MRfZG3RBAboz8AMLTYGrKNkqmVrHt3qtYGA7ht71xfmTK",
        "wif_test_u": "92BJrwB3pGz9r2rRqDTY5PHBzqT8LRtNs376DsxBFmCwaNoc4Px",
        "hex": "03c4bccbc63026e871efdeec3a1a398da8e14a218c5a4b79be06ad74a681baa528",
        "hex_u": "04c4bccbc63026e871efdeec3a1a398da8e14a218c5a4b79be06ad74a681baa528bb79438075570a8beb290e1d606fcefea5f5b29077066fbfe62d2119242a9173",
    },
    {
        "wif": "KxKDsiQBjoyq84AnzKf82Xk5GSPzeHbJB489AZcDnQzGfr3mNfUn",
        "wif_u": "5J4gAgHahWgA6DxEbHTmBmaKe2gCFLcs7MeR45DWda5NDwFxuSg",
        "wif_test": "cNgDLdQ3Asg6HVe4NjUFPrF8tfhQJjgzF6GcGz4jHXeGvb6nmDZx",
        "wif_test_u": "91qJkR78HjkJ4HTXDdMg4N8HHh2uQWA4TJWN8ha1yJpQzv4P3vk",
        "hex": "031b2f91ff071133bcd57f2b91d18773b2858b8f90472c2324f2d64a96d9f118b7",
        "hex_u": "041b2f91ff071133bcd57f2b91d18773b2858b8f90472c2324f2d64a96d9f118b7e1be546bd75d5cdc539ead9611f3128cfe3b1143b2ac8a3fc5cf21e93c2893b1",
    },
    {
        "wif": "Kz2PvtUtw52z7yLxpXUNftHU6mzBg1VTd5cAGSX7xyVmy1xWRuyK",
        "wif_u": "5JT9NGk2LxHmD15XKUJ5TUw2cHzMzmyHuafn9Uz81uDs4gnbFAG",
        "wif_test": "cQPPPoUkN8jFHQpECwHW3CnXj1HbLTb9h7kdNrydU69nDm2edv2V",
        "wif_test_u": "92Dmx1ZZwBMuB4aowpBzL5UzFxM59wWVFXXjE7LdMdxuqh8bDXF",
        "hex": "03d788d2fd66c4a9db40482014637aa8e81dfc3937bd2a98517ca9564d0013bec0",
        "hex_u": "04d788d2fd66c4a9db40482014637aa8e81dfc3937bd2a98517ca9564d0013bec0670c97a9b3bb8f93f248f755db87da6ff09f5d0d3ed581b6fa44016e6000f71b",
    },
    {
        "wif": "L2VtEHPMEp9nLLpwWHRHX19xi1N1inMna231WqL1noXLdpvNSVeo",
        "wif_u": "5K1etVdnNFm8Z2KZjtJHRvWDd2Sc4874ysXpF1XbAUJkRDthjy8",
        "wif_test": "cSrshCPCfsr3VnJCthEQtKf2LEfRPETUe4BUdFnXHvBLta5Fuk7K",
        "wif_test_u": "92nHUETKxUqGX5prNECCJX4BGgoKDHeGKpPmKdt6WD3oCG7qmvT",
        "hex": "0333ec67e454b6a8cf7b7cfa4f0939a9fce934114911f2516b5042d178e8a9850d",
        "hex_u": "0433ec67e454b6a8cf7b7cfa4f0939a9fce934114911f2516b5042d178e8a9850d8829c71f9ee15d1b1fa01f0fd0383c6a5c91da0507402a03bfc4f1db70d929bd",
    },
    {
        "wif": "L4EZ2ismHBC8KtCDbdQwsNiYRNkHiPpRaDtDzv3hodDWsDpPfLpV",
        "wif_u": "5KQTjz4oHMSBGcALcwUBAbpvnrLKjukNRkFKoer6cwRaxMUYaxc",
        "wif_test": "cUbYVdsciEtPVKfUz3E5EhDc3c3hNqv7eG2h7LWDJjsX7xxu5J3y",
        "wif_test_u": "93B6KitLsaWKEffdFHN63CNtSWh2u5HZmh7GtHCbxgAdjUHMFuD",
        "hex": "03f46c9e6d1c63ff6a36cc16c758ae62dbe9b1d5038846be9f542ccc99f8e4cc9f",
        "hex_u": "04f46c9e6d1c63ff6a36cc16c758ae62dbe9b1d5038846be9f542ccc99f8e4cc9fa512b61823d2758d0262c0d1661bf0c546b04a580eaf95a02fcf93bc59519801",
    },
    {
        "wif": "L1Kc4zFKB5bqghB2SoufWA18ct9BNqMAE1XqBbfzs9dKz7Gxi5JX",
        "wif_u": "5JkBfWjfZC1eK4DEvRg7upohhJUgWR6UTxraaDPRth1ys6aewwT",
        "wif_test": "cRgbXuFAc9J6r8eHqDinsUWCF7Sb3HSrJ3gJJ28WNGHLErMczhta",
        "wif_test_u": "92WpFFZD9R5nH7iXYma2nRMfLxqPfadfouiXeqjwERm2eB3MWAc",
        "hex": "0393597b520f59edc837f712f045986333f2c07a2ee6f5090822c5670b9d3f4882",
        "hex_u": "0493597b520f59edc837f712f045986333f2c07a2ee6f5090822c5670b9d3f4882311360e4e2ca12d79f64e0af318680e76a6ac549a2db7a15c68c42e3c600a6e7",
    },
    {
        "wif": "L2u8RxPt7EnzMLgnK5UrXVyUh8JnCVLuGJeVN83uGxCSBHKfqEaP",
        "wif_u": "5K6vLheMeDBsby7kLxPd9GdMmtVx6mwMT8ArUScYnJVRHT6Xirh",
        "wif_test": "cTG7tsPjYJVFWnA3hVHytpUYKMcBrwSbLLnxUYWQn4rSS2NZffk3",
        "wif_test_u": "92sYvSTuESG1a2d2yJHY1sBKRYrfFwUYo52oZ4y483EU4UB8uw4",
        "hex": "039b628957a23699ac4fc01c24b870ef281fe58e2c4f380fc49606333ee13d1c85",
        "hex_u": "049b628957a23699ac4fc01c24b870ef281fe58e2c4f380fc49606333ee13d1c85143a8dec32b8c98619c2d1bdfc7af8623b2dbe1b2c296cc0a6e8aa2526a02a59",
    },
    {
        "wif": "L4ZkzcBhWuEhkJQ5xcfiBzZ6einm27bKFtiC5aBjvHTQMmkbxcse",
        "wif_u": "5KUp8ALBzXQwzqoTxCFfLFrQwK5UeQhQAid4sRjmAMVe1ivq6pR",
        "wif_test": "cUvkTXBYwxvxujsMM2UqZK4AGx6AgZh1KvrfBzeFRQ7QcWqirMJc",
        "wif_test_u": "93FShu9jakV5xuJkaY9aCrQNaySBoaEbWfV1x46GW6Egnkh2WRp",
        "hex": "0271b67404b57c70234ab89f92465dfb6aca0087e27471398237452e60fb5313a5",
        "hex_u": "0471b67404b57c70234ab89f92465dfb6aca0087e27471398237452e60fb5313a55a34b0213e49fd123ab9dfb3eedfbb9ee416f275bb903660d0fe06e5366b575c",
    },
    {
        "wif": "KzZpAtMLUCMXdsGUa9ZXXGN655H7fa9ddrPv5AfiBuNqN7yY8K2g",
        "wif_u": "5JaGDfNNdXDMY1WnfmzDMT8x56W2WanuhyWD2Q2yTVBPHbDjWQ1",
        "wif_test": "cQvodoMBuG3noJjjxZNetas9hJaXL2FKhtYPBb8Dh22qcsAcacYk",
        "wif_test_u": "92LtoQBvDkHVW525J7t8E3guikrjfkL73vNA72PUoDvS4gyPGnS",
        "hex": "022a25f34f58846f738a2aa309ab387beafca0a410a31deed06a21bca1b420f8d1",
        "hex_u": "042a25f34f58846f738a2aa309ab387beafca0a410a31deed06a21bca1b420f8d17b4ab7383ca39bf0b2d24c244872070f25df5fb1ab88bb59502d70708aae78c2",
    },
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
        self.btk.arg("--grep=\"03aeed4c\"")

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