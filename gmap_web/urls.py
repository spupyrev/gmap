from django.conf.urls import patterns, include, url

import views

urlpatterns = patterns('',
	url(r'^admin$', views.home, name='home'),
	url(r'^admin/reload/$', views.rld, name='rld'),
	url(r'^', include('maps.urls', namespace="maps")),
	url(r'^accounts/login/$', 'django.contrib.auth.views.login'),
)
