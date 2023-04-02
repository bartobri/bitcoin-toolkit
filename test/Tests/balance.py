import sys
import json
import unittest
from .btk import BTK

inputs = [
    {
        "temp": "temp",
    }
]

class Balance(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Balance)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("balance")

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

    def test_0010(self):
        pass