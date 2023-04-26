import sys
import json
import unittest
from .btk import BTK

inputs = [
    "seed.bitcoin.sipa.be",
]

class Node(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Node)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("node")

    def io_test(self, opts):
        self.btk.reset()

        for opt in opts:
            self.btk.arg(opt)

        out = self.btk.run()

        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)

        out_text = out.stdout
        out_text = out_text.strip()

        self.assertTrue(out_text)
        self.assertTrue(out_text[0] == "[" or out_text[0] == "{")

        out_json = json.loads(out_text)

        assert(out_json["version"])
        assert(out_json["services"])
        assert(out_json["timestamp"])
        assert(out_json["nonce"])
        assert(out_json["user_agent"])
        assert(out_json["start_height"])
    
    ###########
    ## Tests
    ###########

    def test_0010(self):
        for hostname in inputs:
            self.io_test(opts=[f"--hostname {hostname}"])

    def test_0020(self):
        for hostname in inputs:
            self.io_test(opts=[f"--hostname {hostname}", "--port 8333"])