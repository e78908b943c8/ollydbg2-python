from breakpoints import *
from threads import *
from utils import *
from memory import *
from sym import *
import sys


class Stream(object):
    def __getattr__(self, n):
        return self.orig_stream.__getattr__(n)


class OutStream(Stream):
    # original streams should be initialized statically
    orig_stream = sys.stdout


class ErrStream(Stream):
    orig_stream = sys.stderr


class StandardStream(object):
    def __init__(self):
        self.stdout = OutStream.orig_stream
        self.stderr = ErrStream.orig_stream


class OllyStream(object):
    class OutStream(OutStream):
        @staticmethod
        def write(s):
            """
            This is the hook method for the stdout stream, it tries to
            respect the '\n' in your strings in order to have a clean
            output in the ollydbg log window
            """
            # yeah, each time you do a print 'something' python call two times
            # sys.stdout.write ; first with the string 'something' and the second
            # with only an '\n'
            if len(s) == 1 and s[0] == '\n':
                return

            # now ollydbg just don't care if you have an '\n' in your string
            # it will just display the string added to the list on a single line
            # whatever the string is.
            # The trick is to do other calls to addtolist to emulate a '\n'
            chunks = s.split('\n')
            for substr in chunks:
                Addtolist(0, 0, '%s' % substr)


    class ErrStream(ErrStream):
        def __init__(self):
            super(OllyStream.ErrStream, self).__init__()
            self.buffer = ''

        def write(self, s):
            """
            Trying to bufferize the stream in order to have a proper
            output in the ollydbg log window.

            Each time you call Addtolist it displays a new line, so we keep
            a global buffer until we find an '\n' ; if we found one we display
            the whole buffer, if not we just concatenate the string to the global
            buffer.
            """
            idx = s.find('\n')
            if idx != -1:
                if len(s) == 1:
                    Addtolist(0, 0, '%s' % self.buffer)
                    self.buffer = ''
                else:
                    self.buffer += s[:idx]
                    Addtolist(0, 0, '%s' % self.buffer)
                    if idx != len(s):
                        self.buffer = s[idx + 1:]
                    else:
                        self.buffer = ''
            else:
                # We keep in memory the buffer until finding an '\n'
                # in order to have a clean output in the log window
                self.buffer += s

    def __init__(self):
        self.stdout = self.OutStream()
        self.stderr = self.ErrStream()


def set_stream(stream):
    sys.stdout = stream.stdout
    sys.stderr = stream.stderr


set_stream(OllyStream())


'''
class FileStream(object):
    class WriteMixin(object):
        def write(self, inp):
            return self.open(self.fname).write(inp)

    class OutStream(OutStream, WriteMixin):
        def __init__(self, fname):
            self.fname = fname
            super(FileStream.OutStream, self).__init__()

    class ErrStream(ErrStream, WriteMixin):
        def __init__(self, fname):
            self.fname = fname
            super(FileStream.ErrStream, self).__init__()

    def __init__(self, fname==''):
        self.stdout = self.OutStream(fname)
        self.stderr = self.ErrStream(fname)
'''