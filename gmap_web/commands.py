from os import getpid, kill, wait
from signal import SIGINT
from django.core.management import call_command
from StringIO import StringIO

def restart_django():
    pid = getpid()
    kill(pid, SIGINT)
    return pid

def syncdb():
    output = StringIO()
    call_command('syncdb', interactive=False, stdout=output)
    output.seek(0)
    return output.read()
