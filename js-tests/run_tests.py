from os import listdir
from os.path import isfile, join
import subprocess
import json

import sys

use_file_filter = False
filter = ""
if(len(sys.argv) > 1):
    use_file_filter = True
    filter = sys.argv[1]


test_dir = "/Users/andrewesterhuizen/dev/js/js-tests"
cli_path = "/Users/andrewesterhuizen/dev/js/cmake-build-debug/js"

exluded_files = ["assert.js"]


def is_test_file(file_name):
    return file_name.endswith(".js") and f not in exluded_files


def run_test_file(file_name):
    arg = f"--files={test_dir}/assert.js,{test_dir}/{file_name}"
    result = subprocess.run([cli_path, arg], capture_output=True)
    test_result = json.loads(result.stdout)
    return test_result


file_results = []

for f in listdir(test_dir):
    if isfile(join(test_dir, f)) and is_test_file(f):
        if use_file_filter and not f.startswith(filter):
            continue

        result = run_test_file(f)
        file_results.append(result)


class Colours:
    green = '\033[92m'
    red = '\033[31m'
    end = '\033[0m'


def print_with_color(text, colour):
    print(f"{colour}{text}{Colours.end}")


def print_green(text): return print_with_color(text, Colours.green)
def print_red(text): return print_with_color(text, Colours.red)


n_test_sections = len(file_results)
n_tests = 0
for file_result in file_results:
    for result in file_result["results"]:
        n_tests = n_tests + 1


print(f"// running {n_tests} tests in {n_test_sections} sections:\n")

for file_result in file_results:
    section_name = file_result["section"]
    print(f"// {section_name}")
    for result in file_result["results"]:
        test_name = result["test"]
        if result["passed"]:
            print_green(f"  - {test_name}")
        else:
            failed_reason = result["message"]
            print_red(f"  - {test_name}")
            print_red(f"      reason: {failed_reason}")

    print()
