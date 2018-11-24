import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import unittest


def parse_arguments():
    parser = argparse.ArgumentParser(description='Test gRPC server/client')
    parser.add_argument('--ip', default='localhost', help='server and client IP')
    parser.add_argument('-p', '--port', default='50051', help='server and client port')
    parser.add_argument('-b', '--bin', required=True, help='Build artifacts directory')
    parser.add_argument('-s', '--size', required=True, help='Size of the file to transfer in bytes')
    test_args, suite_args = parser.parse_known_args()
    return test_args, sys.argv[:1] + suite_args


class ClientTestCase(unittest.TestCase):

    def __init__(self, testname, args):
        super(ClientTestCase, self).__init__(testname)
        self.args = args
        self.wd = os.getcwd()
        self.max_block_size = 1024 * 30 # 30MB
        self.__create_test_environment()

    def setUp(self):
        self.__run_server()

    def tearDown(self):
        self.__terminate_server()

    def test_invalid_request(self):
        """
        The client requests a file that the server does not own.
        Check that the file is not received.
        """
        print("Testing invalid request...")
        invalid_filename = os.path.join( \
            os.path.dirname(self.client_path), "7xEvjAeobu")
        os.chdir(os.path.dirname(self.client_path))
        subprocess.call([self.client_path, \
                "{}:{}".format(self.args.ip, self.args.port), \
                invalid_filename])
        self.assertFalse(os.path.isfile(invalid_filename))

    def test_valid_request(self):
        """
        The client requests a file that the server does own.
        Check that the file is received and the content is the same.
        """
        print("Testing valid request...")
        # create a file with random data in the server folder
        self.__create_test_file()
        valid_path = os.path.join(os.path.dirname(self.client_path), \
            os.path.basename(self.test_file))
        os.chdir(os.path.dirname(self.client_path))
        subprocess.call([self.client_path, \
                "{}:{}".format(self.args.ip, self.args.port), \
                os.path.basename(valid_path)])
        self.assertTrue(os.path.isfile(valid_path))
        self.__compare_files(valid_path, self.test_file)
        os.remove(valid_path)

    def __run_server(self):
        """
        Runs the server in a separate process.
        """
        os.chdir(os.path.dirname(self.server_path))
        self.server_process = subprocess.Popen([self.server_path, \
            "{}:{}".format(self.args.ip, self.args.port)])

    def __terminate_server(self):
        self.server_process.terminate()
        self.server_process.wait()

    def __create_test_environment(self):
        """
        Creates the directories for the test environment and copy from the build
        artifacts directory server and client applications.
        """
        os.chdir(self.wd)
        temp_dir = tempfile.gettempdir()
        self.test_root = os.path.join(temp_dir, "test-grpc")
        print("Creating testing environment in {}".format(self.test_root))
        if os.path.exists(self.test_root):
            # delete any previous environment
            shutil.rmtree(self.test_root)
        # create root directory
        os.makedirs(self.test_root)
        def copy_app(name):
            app_root = os.path.join(self.test_root, name)
            os.makedirs(app_root)
            filename = "grpc-{}".format(name)
            src = os.path.join(self.args.bin, filename)
            dst = os.path.join(app_root, filename)
            shutil.copy(src, dst)
            return dst
        # copy client and server into the new test environment
        self.server_path = copy_app("server")
        self.client_path = copy_app("client")

    def __create_test_file(self):
        """
        Create a file with random data in the server folder.
        """
        self.test_file = os.path.join(os.path.dirname(self.server_path), "data")
        with open(self.test_file, "ab+") as f:
            n_blocks = int(self.args.size) // self.max_block_size
            for i in range(n_blocks):
                f.write(bytearray(os.urandom(self.max_block_size)))
            remaining = int(self.args.size) % self.max_block_size
            if remaining > 0:
                f.write(bytearray(os.urandom(remaining)))
        self.assertEqual(int(self.args.size), os.path.getsize(self.test_file))
    
    def __compare_files(self, filename1, filename2):
        """
        Compare two files content, account for files that cannot fit into memory.
        """
        self.assertTrue(os.path.isfile(filename1))
        self.assertTrue(os.path.isfile(filename2))
        self.assertEqual(os.path.getsize(filename1), os.path.getsize(filename2))
        with open(filename1, "rb") as f1:
            with open(filename2, "rb") as f2:
                n_blocks = int(self.args.size) // self.max_block_size
                for i in range(n_blocks):
                    self.assertEqual(f1.read(self.max_block_size), \
                        f2.read(self.max_block_size))
                remaining = int(self.args.size) % self.max_block_size
                if remaining > 0:
                    self.assertEqual(f1.read(remaining), \
                        f2.read(remaining))


def main():
    test_args, suite_args = parse_arguments()
    # run unit tests
    test_loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    for test_name in test_loader.getTestCaseNames(ClientTestCase):
        suite.addTest(ClientTestCase(test_name, test_args))
    result = unittest.TextTestRunner().run(suite)
    sys.exit(not result.wasSuccessful())

if __name__ == "__main__":
    main()
