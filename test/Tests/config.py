import shutil 
import os
import sys
import json
import unittest
from .btk import BTK

inputs = [
    {
        "hostname": "test",
    },
]

class Config(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Config)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("config")
        if (os.path.exists("/tmp/.btk")):
            shutil.rmtree("/tmp/.btk")

    def tearDown(self):
        if (os.path.exists("/tmp/.btk")):
            shutil.rmtree("/tmp/.btk")

    def test_0010(self):

        for key in inputs[0].keys():
            self.btk.reset()
            self.btk.arg("--test")
            self.btk.arg(f"--set {key}={inputs[0][key]}")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)

    def test_0020(self):
        
        self.test_0010()

        self.btk.reset()
        self.btk.arg("--test")
        self.btk.arg(f"--dump")
        out = self.btk.run()
        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)
        out_text = out.stdout
        self.assertTrue(out_text)
        self.assertTrue(out_text[0] == "[" or out_text[0] == "{")

        out_text = out_text.strip()
        out_json = json.loads(out_text)

        for key in inputs[0].keys():
            self.assertTrue(out_json[key])
            self.assertTrue(out_json[key] == inputs[0][key])

    def test_0030(self):
        
        self.test_0010()

        for key in inputs[0].keys():
            self.btk.reset()
            self.btk.arg("--test")
            self.btk.arg(f"--unset {key}")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)

            self.btk.reset()
            self.btk.arg("--test")
            self.btk.arg(f"--dump")
            out = self.btk.run()
            self.assertTrue(out.returncode == 0)
            self.assertTrue(out.stdout)
            out_text = out.stdout
            self.assertTrue(out_text)
            self.assertTrue(out_text[0] == "[" or out_text[0] == "{")

            out_text = out_text.strip()
            out_json = json.loads(out_text)

            self.assertFalse(key in out_json)