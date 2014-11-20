from maps.models import Task
from pipeline import call_graphviz, get_graphviz_map, call_graphviz_scale, set_status
from re import sub, search
import time
from datetime import datetime
from time import strftime

import logging
log = logging.getLogger('gmap_web')

def create_task(task_parameters, user_ip):
	# set up new object

	task = Task()
	task.creation_date = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
	task.creation_ip = user_ip

	task.input_dot = task_parameters['dotfile']
	task.vis_type = task_parameters['vis_type']
	task.layout_algorithm = task_parameters['layout_algorithm']
	task.cluster_algorithm = task_parameters['cluster_algorithm']
	task.contiguous_algorithm = 'contiguous_algorithm'
	task.color_scheme = task_parameters['color_scheme']
	task.semantic_zoom = task_parameters.get('semantic_zoom', 'false')
	task.status = 'created'

	task.save()                                     

	return task

def create_map(task, *args):
	# set up new objects
	dot_rep, svg_rep = call_graphviz(task)
	if dot_rep is None or svg_rep is None:
		return

	if task.semantic_zoom == 'true':
		set_status(task, 'semantic zoom construction')

		svg_rep0 = call_graphviz_scale(dot_rep, 4, 10)
		svg_rep0, width, height = strip_dimensions(svg_rep0)
		task.svg_rep0 = svg_rep0

		svg_rep1 = call_graphviz_scale(dot_rep, 3, 20)
		svg_rep1, width, height = strip_dimensions(svg_rep1)
		task.svg_rep1 = svg_rep1

		svg_rep2 = call_graphviz_scale(dot_rep, 2, 40)
		svg_rep2, width, height = strip_dimensions(svg_rep2)
		task.svg_rep2 = svg_rep2

		svg_rep3 = call_graphviz_scale(dot_rep, 1, 80)
		svg_rep3, width, height = strip_dimensions(svg_rep3)
		task.svg_rep3 = svg_rep3

	svg_rep, width, height = strip_dimensions(svg_rep)

	task.dot_rep = dot_rep
	task.svg_rep = svg_rep
	task.width = width
	task.height = height
	task.status = 'completed'
	task.save()      

def get_formatted_map(task, format):
	return get_graphviz_map(task.dot_rep, format)

def strip_dimensions(svg):
    """having width and height attributes as well as a viewbox will cause openlayers to not display the svg propery, so we strip those attributes out"""
    svg = sub('<title>%3</title>', '', svg, count=1)

    match_re = '<svg width="(.*)pt" height="(.*)pt"'
    replacement = '<svg'
    try:
        width, height = map(float, search(match_re, svg).groups())
    except Exception:
        width, height = 0.0, 0.0
    return sub(match_re, replacement, svg, count=1), width, height

