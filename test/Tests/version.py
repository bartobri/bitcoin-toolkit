import json
import unittest
from .btk import BTK


class Version(unittest.TestCase):

    def run_test(self):
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(Version)
        unittest.TextTestRunner().run(suite)

    def setUp(self):
        self.btk = BTK("version")

    def test_0010(self):
        self.btk.reset()
        out = self.btk.run()
        self.assertTrue(out.returncode == 0)
        self.assertTrue(out.stdout)
        out_text = out.stdout
        self.assertTrue(out_text)

        out_json = json.loads(out_text)

        self.assertTrue(out_json[0])