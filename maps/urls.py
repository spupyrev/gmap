from django.conf.urls import patterns, url

from maps import views

urlpatterns = patterns('',
    url(r'^$', views.index, name='index'),
    url(r'^description$', views.description, name='description'),
    url(r'^datasets$', views.datasets, name='datasets'),
    url(r'^about$', views.about, name='about'),
    url(r'^recent$', views.recent, name='recent'),
    url(r'^map/(\d*)/(\w*)$', views.display_map, name='display_map'),
    url(r'^get_map/(\d*)/$', views.get_map, name='get_map'),
    url(r'^get_map_zoomed/(\d*)$', views.get_map_zoomed, name='get_map_zoomed'),
    url(r'^get_task_metadata/(\d*)/$', views.get_task_metadata, name='get_task_metadata'),
    url(r'^request_map/$', views.request_map, name='request_map'),
    url(r'^delete_map/(\d*)/$', views.delete_map, name='delete_map'),
)
