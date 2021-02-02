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
    url(r'^get_json/(\d*)/$', views.get_json, name='get_json'),
    #url(r'^get_adjacency_matrix/(\d*)/$', views.get_adjacency_matrix, name='get_adjacency_matrix'),
    #url(r'^get_mds$', views.get_mds, name='get_mds'),
    url(r'^request_map/$', views.request_map, name='request_map')
    #url(r'^test$', views.test, name='test')
)
