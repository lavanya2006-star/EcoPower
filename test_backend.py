"""
Legacy backend test runner compatibility wrapper.
Refers to main unit test suite in test_app.py.
"""
from tests.test_app import TestEcoPowerApplication

if __name__ == '__main__':
    import unittest
    unittest.main()
