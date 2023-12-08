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

        "p2pkh": "1NZoZSUY4Lfg5yVn5FPHo4NHEUKcGmGkj1",
        "p2pkh_u": "1BVb7TXi2mGbRnbGoosgZi9FXRWHTFzub3",
        "p2pkh_test": "n35krVZWsN6vs5yPnpMfcyac6TvK9kcyLW",
        "p2pkh_test_u": "mr1YQWcgqnhrCu4tXNr4PdMaPR6zMqm786",
        "bech32": "bc1qaj8dzeef28e9qrwa386krprp56fps44gspvvy7",
        "bech32_test": "tb1qaj8dzeef28e9qrwa386krprp56fps44g68hlld",
        "bech32m": "bc1paj8dzeef28e9qrwa386krprp56fps44gwrttvh",
        "bech32m_test": "tb1paj8dzeef28e9qrwa386krprp56fps44gy9schy",
    },
    {
        "wif": "KyqWNV7VzbrzsyhuDQAzo2qPv3YuiJk5Rp9QwRS2m3ThdMudb5fs",
        "wif_u": "5JQgHCMWE3v1syM9CsZdCnjEMB6RBGMBX6F99Fbfv2TtoJgSb4p",
        "wif_test": "cQCVqQ7MRfZG3RBAboz8AMLTYGrKNkqmVrHt3qtYGA7ht71xfmTK",
        "wif_test_u": "92BJrwB3pGz9r2rRqDTY5PHBzqT8LRtNs376DsxBFmCwaNoc4Px",
        "hex": "03c4bccbc63026e871efdeec3a1a398da8e14a218c5a4b79be06ad74a681baa528",
        "hex_u": "04c4bccbc63026e871efdeec3a1a398da8e14a218c5a4b79be06ad74a681baa528bb79438075570a8beb290e1d606fcefea5f5b29077066fbfe62d2119242a9173",

        "p2pkh": "18aM4Jurg1Ecbds4u1woSzgmbdCExTK26e",
        "p2pkh_u": "1F8wyVLeJsB7mJG8H85V7cwKLMQLYVDptg",
        "p2pkh_test": "mo6JMMzqV2fsNkLgcavBGuu6TcnwqR2Muy",
        "p2pkh_test_u": "mueuGYRd7tcNYQjjzh3rwY9eCM13VG6Hsk",
        "bech32": "bc1q2vt4nxs9a62ldd2gp8jf6telc7ulyu67r2xanu",
        "bech32_test": "tb1q2vt4nxs9a62ldd2gp8jf6telc7ulyu67fvawg0",
        "bech32m": "bc1p2vt4nxs9a62ldd2gp8jf6telc7ulyu67agp6m4",
        "bech32m_test": "tb1p2vt4nxs9a62ldd2gp8jf6telc7ulyu67hw6fqx",
    },
    {
        "wif": "KxKDsiQBjoyq84AnzKf82Xk5GSPzeHbJB489AZcDnQzGfr3mNfUn",
        "wif_u": "5J4gAgHahWgA6DxEbHTmBmaKe2gCFLcs7MeR45DWda5NDwFxuSg",
        "wif_test": "cNgDLdQ3Asg6HVe4NjUFPrF8tfhQJjgzF6GcGz4jHXeGvb6nmDZx",
        "wif_test_u": "91qJkR78HjkJ4HTXDdMg4N8HHh2uQWA4TJWN8ha1yJpQzv4P3vk",
        "hex": "031b2f91ff071133bcd57f2b91d18773b2858b8f90472c2324f2d64a96d9f118b7",
        "hex_u": "041b2f91ff071133bcd57f2b91d18773b2858b8f90472c2324f2d64a96d9f118b7e1be546bd75d5cdc539ead9611f3128cfe3b1143b2ac8a3fc5cf21e93c2893b1",

        "p2pkh": "1Gn5f5MFxbHxaCvGBZtMhf4bdghxfoEAvn",
        "p2pkh_u": "1Azn2etLR3Vzz3acPUw3kX3fxmtSg9imFY",
        "p2pkh_test": "mwJ2x8SEmcjDMKPsu8rjXaGvVgJfYj2uqm",
        "p2pkh_test_u": "mqWjKhyKE4wFmA4E73uRaSFzpmV9aLW6JA",
        "bech32": "bc1q45g8mauwsehetrvzc9dsknxe0250nknqg6de08",
        "bech32_test": "tb1q45g8mauwsehetrvzc9dsknxe0250nknqzuk255",
        "bech32m": "bc1p45g8mauwsehetrvzc9dsknxe0250nknqkc278w",
        "bech32m_test": "tb1p45g8mauwsehetrvzc9dsknxe0250nknqu73dua",
    },
    {
        "wif": "Kz2PvtUtw52z7yLxpXUNftHU6mzBg1VTd5cAGSX7xyVmy1xWRuyK",
        "wif_u": "5JT9NGk2LxHmD15XKUJ5TUw2cHzMzmyHuafn9Uz81uDs4gnbFAG",
        "wif_test": "cQPPPoUkN8jFHQpECwHW3CnXj1HbLTb9h7kdNrydU69nDm2edv2V",
        "wif_test_u": "92Dmx1ZZwBMuB4aowpBzL5UzFxM59wWVFXXjE7LdMdxuqh8bDXF",
        "hex": "03d788d2fd66c4a9db40482014637aa8e81dfc3937bd2a98517ca9564d0013bec0",
        "hex_u": "04d788d2fd66c4a9db40482014637aa8e81dfc3937bd2a98517ca9564d0013bec0670c97a9b3bb8f93f248f755db87da6ff09f5d0d3ed581b6fa44016e6000f71b",

        "p2pkh": "1A5gLn9BGfwPYx2HUCKoZBsoHvs7EELMYh",
        "p2pkh_u": "19doM7Pdar6U9uc3P7Yo1YHBb9c7eY1EuM",
        "p2pkh_test": "mpbddqEA5hNeL4VuBmJBP7689vTp4rw2Vy",
        "p2pkh_test_u": "mp9keAUcPsXiw25f6gXAqTVWT9CpedJesA",
        "bech32": "bc1qvwd69k9xa746xt60t22600eq7vzzlrrrecakch",
        "bech32_test": "tb1qvwd69k9xa746xt60t22600eq7vzzlrrrn7x9ry",
        "bech32m": "bc1pvwd69k9xa746xt60t22600eq7vzzlrrr8663s7",
        "bech32m_test": "tb1pvwd69k9xa746xt60t22600eq7vzzlrrrdupztd",
    },
    {
        "wif": "L2VtEHPMEp9nLLpwWHRHX19xi1N1inMna231WqL1noXLdpvNSVeo",
        "wif_u": "5K1etVdnNFm8Z2KZjtJHRvWDd2Sc4874ysXpF1XbAUJkRDthjy8",
        "wif_test": "cSrshCPCfsr3VnJCthEQtKf2LEfRPETUe4BUdFnXHvBLta5Fuk7K",
        "wif_test_u": "92nHUETKxUqGX5prNECCJX4BGgoKDHeGKpPmKdt6WD3oCG7qmvT",
        "hex": "0333ec67e454b6a8cf7b7cfa4f0939a9fce934114911f2516b5042d178e8a9850d",
        "hex_u": "0433ec67e454b6a8cf7b7cfa4f0939a9fce934114911f2516b5042d178e8a9850d8829c71f9ee15d1b1fa01f0fd0383c6a5c91da0507402a03bfc4f1db70d929bd",

        "p2pkh": "1PjNdh3GZDk3W78526cdLEByme5Yn9mMEh",
        "p2pkh_u": "13jYYduezz2Vio8nHs55Jg3YFXqtDh51rt",
        "p2pkh_test": "n4FKvk8FNFBJHDbgjfb1A9QJddgFib6WkN",
        "p2pkh_test_u": "miFVqgzdp1TkVucQ1S3T8bFs7XSb4vQBba",
        "bech32": "bc1ql9tyhtv8p60tq6m29whfge9a7vw2xwnlqdj8fw",
        "bech32_test": "tb1ql9tyhtv8p60tq6m29whfge9a7vw2xwnl2tf5ja",
        "bech32m": "bc1pl9tyhtv8p60tq6m29whfge9a7vw2xwnl704qp8",
        "bech32m_test": "tb1pl9tyhtv8p60tq6m29whfge9a7vw2xwnl5fwn65",
    },
    {
        "wif": "L4EZ2ismHBC8KtCDbdQwsNiYRNkHiPpRaDtDzv3hodDWsDpPfLpV",
        "wif_u": "5KQTjz4oHMSBGcALcwUBAbpvnrLKjukNRkFKoer6cwRaxMUYaxc",
        "wif_test": "cUbYVdsciEtPVKfUz3E5EhDc3c3hNqv7eG2h7LWDJjsX7xxu5J3y",
        "wif_test_u": "93B6KitLsaWKEffdFHN63CNtSWh2u5HZmh7GtHCbxgAdjUHMFuD",
        "hex": "03f46c9e6d1c63ff6a36cc16c758ae62dbe9b1d5038846be9f542ccc99f8e4cc9f",
        "hex_u": "04f46c9e6d1c63ff6a36cc16c758ae62dbe9b1d5038846be9f542ccc99f8e4cc9fa512b61823d2758d0262c0d1661bf0c546b04a580eaf95a02fcf93bc59519801",

        "p2pkh": "1HWdab5xFjvPKbxHKHREcqNJmfuA8t8dbM",
        "p2pkh_u": "18nRy22wEiDQLYo8nyoATX5bDexThthu8E",
        "p2pkh_test": "mx2aseAw4mMe6iRu2rPcSkaddfVs3882Lz",
        "p2pkh_test_u": "moJPG57v3jef7fGkWYmYHSHv5eZAbE5Kwx",
        "bech32": "bc1qk5wfcvfs8rsfj3qch04l75l7t9atp3arjryyx6",
        "bech32_test": "tb1qk5wfcvfs8rsfj3qch04l75l7t9atp3arc9lhaf",
        "bech32m": "bc1pk5wfcvfs8rsfj3qch04l75l7t9atp3arvprrwn",
        "bech32m_test": "tb1pk5wfcvfs8rsfj3qch04l75l7t9atp3arx8cs4q",
    },
    {
        "wif": "L1Kc4zFKB5bqghB2SoufWA18ct9BNqMAE1XqBbfzs9dKz7Gxi5JX",
        "wif_u": "5JkBfWjfZC1eK4DEvRg7upohhJUgWR6UTxraaDPRth1ys6aewwT",
        "wif_test": "cRgbXuFAc9J6r8eHqDinsUWCF7Sb3HSrJ3gJJ28WNGHLErMczhta",
        "wif_test_u": "92WpFFZD9R5nH7iXYma2nRMfLxqPfadfouiXeqjwERm2eB3MWAc",
        "hex": "0393597b520f59edc837f712f045986333f2c07a2ee6f5090822c5670b9d3f4882",
        "hex_u": "0493597b520f59edc837f712f045986333f2c07a2ee6f5090822c5670b9d3f4882311360e4e2ca12d79f64e0af318680e76a6ac549a2db7a15c68c42e3c600a6e7",

        "p2pkh": "1GRir3BomKuaqeRnbRsVGNJz17myAb9w9N",
        "p2pkh_u": "1BQHS5NLvCQ361JWHKdfJDGzXJ3VUZ3zVT",
        "p2pkh_test": "mvwg96GnaMLqckuQJzqs6HXJs7Ng7bev1A",
        "p2pkh_test_u": "mqvEj8TKjDqHs7n7ztc388VKPHeCQDGaBu",
        "bech32": "bc1q4ymve7dnxnqahayq3pr9hum0ujtnf7geq8syus",
        "bech32_test": "tb1q4ymve7dnxnqahayq3pr9hum0ujtnf7ge2pth8r",
        "bech32m": "bc1p4ymve7dnxnqahayq3pr9hum0ujtnf7ge79hr5e",
        "bech32m_test": "tb1p4ymve7dnxnqahayq3pr9hum0ujtnf7ge5rvs02",
    },
    {
        "wif": "L2u8RxPt7EnzMLgnK5UrXVyUh8JnCVLuGJeVN83uGxCSBHKfqEaP",
        "wif_u": "5K6vLheMeDBsby7kLxPd9GdMmtVx6mwMT8ArUScYnJVRHT6Xirh",
        "wif_test": "cTG7tsPjYJVFWnA3hVHytpUYKMcBrwSbLLnxUYWQn4rSS2NZffk3",
        "wif_test_u": "92sYvSTuESG1a2d2yJHY1sBKRYrfFwUYo52oZ4y483EU4UB8uw4",
        "hex": "039b628957a23699ac4fc01c24b870ef281fe58e2c4f380fc49606333ee13d1c85",
        "hex_u": "049b628957a23699ac4fc01c24b870ef281fe58e2c4f380fc49606333ee13d1c85143a8dec32b8c98619c2d1bdfc7af8623b2dbe1b2c296cc0a6e8aa2526a02a59",

        "p2pkh": "1MUazyY9q9fr59FL6m2osUPmGHE29imKUc",
        "p2pkh_u": "1HHamteenaCFdZYhHb9WW2rAsyD5dAo5cB",
        "p2pkh_test": "n1zYJ2d8eB76rFiwpL1BhPc68Gpj3448ha",
        "p2pkh_test_u": "mwoY4wjdbbdWQg2K1A7tKx4VjxonVQXJj5",
        "bech32": "bc1quzdzp3lu5cs72vrl82hpc4kpttg43uy8llj6gt",
        "bech32_test": "tb1quzdzp3lu5cs72vrl82hpc4kpttg43uy84effnc",
        "bech32m": "bc1puzdzp3lu5cs72vrl82hpc4kpttg43uy8pa4aqz",
        "bech32m_test": "tb1puzdzp3lu5cs72vrl82hpc4kpttg43uy8tmwwm3",
    },
    {
        "wif": "L4ZkzcBhWuEhkJQ5xcfiBzZ6einm27bKFtiC5aBjvHTQMmkbxcse",
        "wif_u": "5KUp8ALBzXQwzqoTxCFfLFrQwK5UeQhQAid4sRjmAMVe1ivq6pR",
        "wif_test": "cUvkTXBYwxvxujsMM2UqZK4AGx6AgZh1KvrfBzeFRQ7QcWqirMJc",
        "wif_test_u": "93FShu9jakV5xuJkaY9aCrQNaySBoaEbWfV1x46GW6Egnkh2WRp",
        "hex": "0271b67404b57c70234ab89f92465dfb6aca0087e27471398237452e60fb5313a5",
        "hex_u": "0471b67404b57c70234ab89f92465dfb6aca0087e27471398237452e60fb5313a55a34b0213e49fd123ab9dfb3eedfbb9ee416f275bb903660d0fe06e5366b575c",

        "p2pkh": "162dCwKCKNq5NhX7KnzZbL9x81bcibiJrY",
        "p2pkh_u": "1d7iyuVJ8JUCXACzLnvCFqTfEcKWYgWXm",
        "p2pkh_test": "mkYaVzQB8QGL9ozj3MxwRFNGz1CKYujLCE",
        "p2pkh_test_u": "mg9522zU79jiyddphumJ2B3nXED2U2fbbW",
        "bech32": "bc1qxun5fxgesuczgzvvyermjgd6z0y29mgr8ltggn",
        "bech32_test": "tb1qxun5fxgesuczgzvvyermjgd6z0y29mgrdesmnq",
        "bech32m": "bc1pxun5fxgesuczgzvvyermjgd6z0y29mgreav0q6",
        "bech32m_test": "tb1pxun5fxgesuczgzvvyermjgd6z0y29mgrnmhumf",
    },
    {
        "wif": "KzZpAtMLUCMXdsGUa9ZXXGN655H7fa9ddrPv5AfiBuNqN7yY8K2g",
        "wif_u": "5JaGDfNNdXDMY1WnfmzDMT8x56W2WanuhyWD2Q2yTVBPHbDjWQ1",
        "wif_test": "cQvodoMBuG3noJjjxZNetas9hJaXL2FKhtYPBb8Dh22qcsAcacYk",
        "wif_test_u": "92LtoQBvDkHVW525J7t8E3guikrjfkL73vNA72PUoDvS4gyPGnS",
        "hex": "022a25f34f58846f738a2aa309ab387beafca0a410a31deed06a21bca1b420f8d1",
        "hex_u": "042a25f34f58846f738a2aa309ab387beafca0a410a31deed06a21bca1b420f8d17b4ab7383ca39bf0b2d24c244872070f25df5fb1ab88bb59502d70708aae78c2",

        "p2pkh": "189jLPhwhft7RXFKXptSUfxssY97wVbd7V",
        "p2pkh_u": "1DHLvvuUonDqgZeWVSYMXYqUEBzt1kFXEH",
        "p2pkh_test": "mnfgdSnvWhKNCdiwFPrpJbBCjXjppPKKfu",
        "p2pkh_test_u": "msoJDyzTcof6Tg88D1WjMU3o6Bbb1F2zD3",
        "bech32": "bc1qfehccrcdk2zmgpxx8xew54skpgqj36v3a89pse",
        "bech32_test": "tb1qfehccrcdk2zmgpxx8xew54skpgqj36v3hp7jt2",
        "bech32m": "bc1pfehccrcdk2zmgpxx8xew54skpgqj36v3r9zxcs",
        "bech32m_test": "tb1pfehccrcdk2zmgpxx8xew54skpgqj36v3fre4rr",
    },
]

class Address(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Address)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("address")

    def io_test(self, opts, input, output, input_json=True, input_arg=False, output_json=True):

        for input_group in inputs:
            self.btk.reset()

            if (input):
                if (input_arg):
                    self.btk.arg(input_group[input])
                elif (input_json):
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

    def test_0045(self):
        self.io_test(opts=["-w"], input="wif", output="p2pkh", input_arg=True)

    def test_0050(self):
        self.io_test(opts=["-w", "-l"], input="wif", output="p2pkh", input_json=False)

    def test_0060(self):
        self.io_test(opts=["-w", "--in-format=list"], input="wif", output="p2pkh", input_json=False)

    def test_0070(self):
        self.io_test(opts=["-w", "-j"], input="wif", output="p2pkh")

    def test_0080(self):
        self.io_test(opts=["-w", "--in-format=json"], input="wif", output="p2pkh")

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
        self.io_test(opts=["-w", "--legacy"], input="wif", output="p2pkh")

    def test_0130(self):
        self.io_test(opts=["-w", "--bech32"], input="wif", output="bech32")

    def test_0135(self):
        self.io_test(opts=["-w", "--bech32m"], input="wif", output="bech32m")

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
        self.io_test(opts=["-w", "--legacy"], input="wif_u", output="p2pkh_u")

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
        self.io_test(opts=["-w", "--legacy"], input="wif_test", output="p2pkh_test")

    def test_0220(self):
        self.io_test(opts=["-w", "--bech32"], input="wif_test", output="bech32_test")

    def test_0225(self):
        self.io_test(opts=["-w", "--bech32m"], input="wif_test", output="bech32m_test")

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
        self.io_test(opts=["-w", "--legacy"], input="wif_test_u", output="p2pkh_test_u")

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
        self.io_test(opts=["-x", "--legacy"], input="hex", output="p2pkh")

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
        self.io_test(opts=["-x", "--legacy"], input="hex_u", output="p2pkh_u")

    ####################
    ## Grep
    ####################

    def test_0360(self):

        input = inputs[0]["wif"]

        self.btk.reset()
        self.btk.set_input(f"[\n\"{input}\"\n]")
        self.btk.arg("-w")
        self.btk.arg("--legacy")
        self.btk.arg("--grep=\"1NZoZS\"")

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
        self.btk.arg("--legacy")
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
            self.btk.arg("--legacy")
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
            self.btk.arg("--legacy")
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