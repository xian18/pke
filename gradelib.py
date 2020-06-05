from __future__ import print_function
import os, re, traceback



TOTAL = POSSIBLE = 0

def test_app(app):
    cmd = 'spike build/pk app/' + app+'>> pke_out.txt'
    os.system(cmd)

def test(points, title=None, parent=None):
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
    print("Score: %d/%d" % (TOTAL, POSSIBLE))

COLORS = {"default": "\033[0m", "red": "\033[31m", "green": "\033[32m"}

def color(name, text):
    return COLORS[name] + text + COLORS["default"]
