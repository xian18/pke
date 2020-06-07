from __future__ import print_function
import sys, os, re, traceback



TOTAL = POSSIBLE = 0

def test_app(app):
    cmd = 'spike build/pk app/' + app+'>> pke_out.txt'
    os.system(cmd)

def test(points, title=' ', parent=None):
    """Decorator for declaring test functions.  If title is None, the
    title of the test will be derived from the function name by
    stripping the leading "test_" and replacing underscores with
    spaces."""
    
    def register_test(fn, title=title):
        if parent:
            title = "  " + title
        global TOTAL, POSSIBLE
        fail = None

        print(title, ":", end=' ')
        try:
            fn()
        except AssertionError as e:
            fail = "".join(traceback.format_exception_only(type(e), e))
        
        POSSIBLE += points
        
        if fail:
            print(color("red", "FAIL"))
            print("    %s" % fail.replace("\n", "\n    "))
        else:
            print(color("green", "OK"))
            TOTAL += points
    return register_test


def show_grade():
    print()
    print("Score: %d/%d" % (TOTAL, POSSIBLE))

COLORS = {"default": "\033[0m", "red": "\033[31m", "green": "\033[32m"}

def color(name, text):
    return COLORS[name] + text + COLORS["default"]

class Runner():
    pke_out = ""
    def run_app(self, app):
        cmd = 'riscv64-unknown-elf-gcc -o app/' + app +' app/'+ app + '.c'
        ret = os.system(cmd)
        if ret != 0:
            print(color('red', 'running app error!'))
            sys.exit(1)
        cmd = 'spike build/pk app/' + app+'> pke_out.txt'
        os.system(cmd)
        self.get_pke_out()
        
    def get_pke_out(self):
        with open("pke_out.txt", "r") as f:  
            self.pke_out = f.read()