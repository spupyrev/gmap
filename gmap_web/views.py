from django.contrib.auth.decorators import login_required
from django.http import HttpResponse

import commands

@login_required
def rld(request):
    pid = commands.restart_django()
    return HttpResponse(u'killed process %s' % pid, content_type='text/plain')

@login_required
def syncdb(request):
    output = commands.syncdb()
    return HttpResponse(output, content_type='text/plain')

@login_required
def home(request):
    return HttpResponse(u'Django is working.')
