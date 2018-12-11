from django.shortcuts import render, redirect, render_to_response, get_object_or_404
from django.http import HttpResponse

from lib.web_interface import create_task, create_map, get_formatted_map
from lib.interface import CallExternalException
from maps.models import Task
import thread
import time
import json
import pygraphviz
from time import strftime
import networkx as nx
from networkx.readwrite import json_graph
from networkx.drawing import nx_agraph
from lib.sphere_mds import dot_to_adjacency_matrix

def index(request):
	return render(request, 'maps/index.html')

def description(request):
	return render(request, 'description.html')

def datasets(request):
	return render(request, 'datasets.html')

def about(request):
	return render(request, 'about.html')

def recent(request):
	map_list = []

	debug = request.GET.get('debug', 'false')
	page = int(request.GET.get('page', '0'))
	items_per_page = 30
	start = items_per_page*page
	end = start + items_per_page

	for task in Task.objects.order_by('-id')[start:end]:
		obj = {}
		obj['id'] = task.id
		obj['link'] = '/map/' + str(task.id)
		if len(task.input_dot) < 75:
			obj['dot'] = task.input_dot
		else:
			obj['dot'] = task.input_dot[:75] + '...'
		obj['dot_link'] = '/map/' + str(task.id) + '/input_dot'
		obj['date'] = task.creation_date
		obj['ip'] = task.creation_ip
		obj['status'] = task.status
		obj['status_link'] = '/map/' + str(task.id) + '/input_desc'
		map_list.append(obj)
	return render(request, 'maps/recent.html', {'map_list': map_list, 'debug': debug})


def request_map(request):
	if request.method == 'POST':
		try:
			task = create_task(dict(request.POST.items()), request.META.get('REMOTE_ADDR', ''))
			try:
				thread.start_new_thread(create_map, (task, 1))
			except Exception as e:
				print 'thread-error: ' + str(e)
			return redirect('/map/' + str(task.id))
		except CallExternalException as e:
			return render(request, 'maps/error.html', {'msg': str("<br />".join(str(e).split("\n")))})


MIME_TYPES = {
    'pdf': 'application/pdf',
    'ps':  'application/postscript',
    'eps': 'application/postscript',
    'png': 'image/png',
    'gif': 'image/gif',
    'jpg': 'image/jpeg',
    'svg': 'image/svg+xml',
    'dot': 'text/plain',
    'gv':  'text/plain',
}

def display_map(request, task_id, format = ''):
	if request.method == 'GET':
		task = get_object_safe(task_id, 10)
		if format == '':
			if task.spherical:
				return render(request, 'maps/spherical.html', {'task': task})
			return render(request, 'maps/map.html', {'task': task})
		else:
			if format in MIME_TYPES:
				content = get_formatted_map(task, format)
				return HttpResponse(content, content_type = MIME_TYPES[format])
			elif format == 'input_dot':
				return HttpResponse(task.input_dot, 'text/plain')
			elif format == 'input_desc':
				return HttpResponse(task.description(), 'text/plain')

def get_json(request, task_id):
	if request.method == 'GET':
		task = Task.objects.get(id = task_id)
		dot_graph = nx_agraph.from_agraph(pygraphviz.AGraph(task.dot_rep))
  		graph_json = json.dumps(json_graph.node_link_data(dot_graph))
		return HttpResponse(graph_json, content_type='application/json')

def get_adjacency_matrix(request, task_id):
	if request.method == 'GET':
		task = Task.objects.get(id = task_id)
		print dot_to_adjacency_matrix(pygraphviz.AGraph(task.dot_rep))
		

def get_task_metadata(request, task_id):
    if request.method == 'GET':
		task = Task.objects.get(id = task_id)
		return HttpResponse(task.json_metadata(), content_type='application/json')
    
def get_map(request, task_id):
    if request.method == 'GET':
		task = Task.objects.get(id = task_id)
		return HttpResponse(u'%s' % task.svg_rep)

import logging
log = logging.getLogger('gmap_web')

def get_map_zoomed(request, task_id):
    if request.method == 'GET':
		zoom = request.GET.get('zoom', '3')
		task = get_object_safe(task_id, 10)
		if zoom == '0':
			return HttpResponse(u'%s' % task.svg_rep0)
		elif zoom == '1':
			return HttpResponse(u'%s' % task.svg_rep1)
		elif zoom == '2':
			return HttpResponse(u'%s' % task.svg_rep2)
		elif zoom == '3':
			return HttpResponse(u'%s' % task.svg_rep3)

def get_object_safe(task_id, attempt):
	if attempt <= 0:
		return get_object_or_404(Task, id = task_id)

	try:
		task = get_object_or_404(Task, id = task_id)
		return task
	except Exception as e:
		if type(e).__name__ != 'OperationalError':
			raise e
		print type(e).__name__ + ': ' + str(e)
		time.sleep(0.1)
		return get_object_safe(task_id, attempt - 1)
            