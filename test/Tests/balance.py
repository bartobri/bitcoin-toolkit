import pathlib
import json
import unittest
from .btk import BTK

inputs = [
    {
        "12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX": "5000000000",
        "1HLoD9E4SDFFPDiYfNYnkBLQ85Y51J3Zb1": "5000000000",
        "1FvzCLoTPGANNjWoUo6jUGuAG3wg1w4YjR": "5000000000",
        "15ubicBBWFnvoZLT7GiU2qxjRaKJPdkDMG": "5000000000",
        "1JfbZRwdDHKZmuiZgYArJZhcuuzuw2HuMu": "5000000000",

    },
    {
        "1testSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX": "0",
        "1test9E4SDFFPDiYfNYnkBLQ85Y51J3Zb1": "0",
        "1testLoTPGANNjWoUo6jUGuAG3wg1w4YjR": "0",
        "1testcBBWFnvoZLT7GiU2qxjRaKJPdkDMG": "0",
        "1testRwdDHKZmuiZgYArJZhcuuzuw2HuMu": "0",

    }
]

class Balance(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Balance)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("balance")
        self.balance_path = pathlib.Path(__file__).parent.parent.joinpath('balance').resolve()

    def io_test(self, opts, input_json=True, input_arg=False, output_json=True):

        for input_group in inputs:
            for key in input_group.keys():
                self.btk.reset()

                if (input_arg):
                    self.btk.arg(key)
                elif (input_json):
                    self.btk.set_input(f"[\n\"{key}\"\n]")
                else:
                    self.btk.set_input(key)

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

                out_text = out_text.strip()

                self.assertTrue(out_text == input_group[key])

    ###########
    ## No Opts
    ###########

    def test_0010(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}"])

    ################
    ## Output Format
    ################

    def test_0020(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "-J"])

    def test_0030(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "--out-format=json"])

    def test_0040(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "-L"], output_json=False)

    def test_0050(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "--out-format=list"], output_json=False)

    ################
    ## Input Format
    ################

    def test_0055(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}"], input_arg=True)

    def test_0060(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "-j"])

    def test_0070(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "--in-format=json"])

    def test_0080(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "-l"], input_json=False)

    def test_0090(self):
        self.io_test(opts=[f"--balance-path={self.balance_path}", "--in-format=list"], input_json=False)

    ########
    ## Grep
    ########

    def test_0100(self):
        for key in inputs[0]:
            self.btk.reset()
            self.btk.set_input(key)
            self.btk.arg(f"--balance-path={self.balance_path}")
            self.btk.arg(f"--grep={inputs[0][key]}")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            out_text = out_text.strip()
            self.assertTrue(out_text)
            self.assertTrue(out_text[0] == "[" or out_text[0] == "{")
            out_json = json.loads(out_text)
            out_text = out_json[0]
            self.assertTrue(out_text)

    def test_0110(self):
        for key in inputs[0]:
            self.btk.reset()
            self.btk.set_input(key)
            self.btk.arg(f"--balance-path={self.balance_path}")
            self.btk.arg(f"--grep=nomatch")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertFalse(out.stdout)