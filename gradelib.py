from __future__ import print_function
import sys, os, re, traceback

dir = '/data/workspace/myshixun/src/step1/'
#dir='./'
TOTAL = POSSIBLE = 0

def test_app(app):
    cmd = 'spike build/pk '+dir+'/app/' + app+'>> pke_out.txt'
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
            print("    %s" % fail)
        #    print("    %s" % fail.replace("\n", "\n    "))
        else:
            print(color("green", "OK"))
            TOTAL += points
    return register_test


def show_grade():
    print("Score: %d/%d" % (TOTAL, POSSIBLE))

COLORS = {"default": "", "red": "", "green": ""}


def color(name, text):
    return COLORS[name] + text + COLORS["default"]

class Runner():
    pke_out = ""
    def run_build_pk(self):
        cmd = 'make -C '+dir+' > pke_out.txt'
        ret = os.system(cmd)
        if ret != 0:
            print(color('red', 'build pk error!'))
            sys.exit(1)
    def run_app(self, app, mem = '2048'):
        cmd = 'riscv64-unknown-elf-gcc -o '+dir+'app/elf/' + app +' '+dir+'app/'+ app + '.c'
        ret = os.system(cmd)
        if ret != 0:
            print(color('red', 'running app error!'))
            sys.exit(1)
        cmd = 'spike -m'+mem +' '+dir+'obj/pke '+dir+'app/elf/' + app+' > pke_out.txt'
        os.system(cmd)
        self.get_pke_out()
        
    def get_pke_out(self):
        with open("pke_out.txt", "r") as f:  
            self.pke_out = f.read()
    def match(self, *args, **kwargs):
        assert_lines_match(self.pke_out, *args, **kwargs)


def assert_lines_match(text, *regexps, **kw):
    """Assert that all of regexps match some line in text.  If a 'no'
    keyword argument is given, it must be a list of regexps that must
    *not* match any line in text."""

    def assert_lines_match_kw(no=[]):
        return no
    no = assert_lines_match_kw(**kw)

    # Check text against regexps
    lines = text.splitlines()
    good = set()
    bad = set()
    for i, line in enumerate(lines):
        if any(re.search(r, line) for r in regexps):
            good.add(i)
            regexps = [r for r in regexps if not re.search(r, line)]
        if any(re.search(r, line) for r in no):
            bad.add(i)

    if not regexps and not bad:
        return
    # We failed; construct an informative failure message
    show = set()
    for lineno in good.union(bad):
        for offset in range(-2, 3):
            show.add(lineno + offset)
    if regexps:
        show.update(n for n in range(len(lines) - 5, len(lines)))

    msg = []
    last = -1
    for lineno in sorted(show):
        if 0 <= lineno < len(lines):
            if lineno != last + 1:
                msg.append("...")
            last = lineno
            msg.append("%s %s" % (color("red", "BAD ") if lineno in bad else
                                  color("green", "GOOD") if lineno in good
                                  else "    ",
                                  lines[lineno]))
    if last != len(lines) - 1:
        msg.append("...")
    if bad:
        msg.append("unexpected lines in output")
    for r in regexps:
        msg.append(color("red", "MISSING") + " '%s'" % r)
    raise AssertionError("\n".join(msg))


