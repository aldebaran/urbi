#!/usr/bin/python

import optparse
import os
import re
import sys
from tempfile import NamedTemporaryFile

## ------------- ##
## Parse options ##
## ------------- ##

usage="usage: %s [options] input.tex"

parser = optparse.OptionParser(usage=usage % sys.argv[0])

opts, args = parser.parse_args()

if len(args) != 1:
    print >> sys.stderr, usage
    exit(1)
input_file = sys.argv[1]
urbi_console = os.getenv('URBI_CONSOLE')

## ------------- ##
## Extract tests ##
## ------------- ##

tests = []

test = ""
testing = False
skip = False
start = None
n = 0
for line in open(input_file, 'r'):
    n += 1
    if testing:
        if skip:
            skip = False
            continue
        if re.compile('\\\\end{urbiscript}').match(line):
            testing = False
            tests.append((start, test))
            test = ""
        else:
            test += line
    else:
        if re.compile('\\\\begin{urbiscript}').match(line):
            testing = True
            start = n
            if re.compile('\\\\begin{urbiscript}\\[').match(line) \
                   and not re.compile('.*\\]').match(line):
                skip = True


def splitTest(test):

    in_ = ""
    out_ = ""

    for line in test[1].split('\n'):
        rg = re.compile('^\s*\[\d{8}(:\w+)?\]')
        rg2 = re.compile('^\s*\.\.')
        if rg.match(line) or rg2.match(line):
            line = line.strip()
            line = rg.sub('[00000000]', line)
            line = rg2.sub('', line)
            out_ += '%s\n' % line
        else:
            in_ += '%s\n' % line

    return (test[0], in_, out_)

tests = map(splitTest, tests)

## ----- ##
## Check ##
## ----- ##

# FIXME: do this with uconsole-check

def diff(line, f1, f2):

    res = os.system('diff -qu %s %s > /dev/null' % (f1, f2))
    print '%s: %s:' % (input_file, line),
    if os.WIFEXITED(res) and os.WEXITSTATUS(res) == 0:
        print 'PASS'
        return True
    else:
        print 'FAIL'
        os.system('diff -u %s %s | sed "s/^/\t/"' % (f1, f2))
        return False


res = 0

for (n, in_, out_) in tests:

    ferr = NamedTemporaryFile()
    fexp = NamedTemporaryFile()
    fin = NamedTemporaryFile()
    fout = NamedTemporaryFile()

    f = open(fin.name, 'w')
    print >> f, '{sleep(5s); echo("TIMEOUT!"); shutdown},'
    print >> f, in_,
    print >> f, 'shutdown;'
    f.close()

    f = open(fexp.name, 'w')
    print >> f, out_,
    f.close()

#     print "INPUT"
#     os.system("cat %s" % fin.name)
#     print "/INPUT"

    st = os.system('%s --fast %s > %s 2> %s' % (urbi_console, fin.name, fout.name, ferr.name))
    if os.WIFSIGNALED(st):
        exit(1) # Signal

    found = False

    lines = list(open(fout.name))

    start = 1
    for line in lines:
        if re.search('"Urbi is up and running."', line):
            break
        start = start + 1

    lines = lines[start:]

    def rewrite(line):
        line = re.sub(r'\[\d{8}(:\w+)?]', '[00000000]', line)
        line = re.sub('0x[0-9a-fA-F]+', '0xADDR', line)
        line = re.sub('!!! /[^:]*:[^:]*: ', '!!! ', line)
        return line

    def remove(line):
        return not re.match(r'\[\d{8}\] !!!    called from', line)

    lines = map(rewrite, lines)
    lines = filter(remove, lines)

    f = open(fout.name, 'w')
    print >> f, ''.join(lines),
    f.close()

    if not diff (n, fexp.name, fout.name):
        res = 1

exit(res)


# for (in_, out_) in tests:
#     print '<<<<<<<<<<'
#     print in_
#     print '----------'
#     print out_
#     print '>>>>>>>>>>'
