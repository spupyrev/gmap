#!/usr/bin/python
import threading
threading._DummyThread._Thread__stop = lambda x: 42
from subprocess import Popen, PIPE
from interface import CallExternalException
from maps.models import Task

import logging
log = logging.getLogger('gmap_web')

import os
CURPATH = os.path.dirname(__file__)
CURPATH = os.path.normpath(os.path.join(CURPATH, os.pardir))
if CURPATH.startswith('/cygdrive'):
	path = CURPATH.split('/')
	CURPATH = path[2] + ":/" + "/".join(path[3:])

def graphviz_command_layout(alg='sfdp'):
    return "%s -Goverlap=prism -Goutputorder=edgesfirst -Gsize=60,60!" % (alg)

def graphviz_command_gmap(color_scheme):
    if color_scheme == 'bubble-sets':
    	return "gvmap -e  -s -4"
    return "gvmap -e  -s -4 -c %s" % (color_scheme)

def graphviz_command_draw(file_format='svg'):
    return "neato -Gforcelabels=false -Ecolor=grey  -Gsize=60,60! -n2 -T%s" % (file_format)

def graphviz_command_scale(s1, s2):
    script = CURPATH + "/external/utils/change_size.gvpr"
    return "gvpr -c -a %s -f %s | neato -Gsize=%s! -Ecolor=grey -n2 -Tsvg" % (s1, script, s2)

def cluster_command(alg):
    return CURPATH + "/external/eba/kmeans -action=clustering -C=%s" % (alg)

def ceba_command():
    return "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:" + CURPATH + "/external/ecba/libraries/tulip/install/lib; " + CURPATH + "/external/ecba/build/Exec -p -r"

def bubblesets_command():
    script = CURPATH + "/external/BubbleSets.jar"
    return "java -cp " + script + " setvis.Main -p -r"
    
def colors_command():
    script = CURPATH + "/external/BubbleSets.jar"
    return "java -cp " + script + " setvis.Main -p -r -c"
    
def linesets_command():
    script = CURPATH + "/external/LineSets.jar"
    return "java -cp " + script + " setvis.Main -p -r"
    
def mapsets_command():
    return CURPATH + "/external/mapsets/mapsets"
    
def mapsets_post_command(color_scheme):
    if color_scheme == 'bubble-sets':
    	return "gvmap -e -a 0 -s 200"
    return "gvmap -e -a 0 -s 200 -c %s" % (color_scheme)

def pointcloud_command():
    return CURPATH + "/external/pointcloud/pointcloud"
    

def removeNonAscii(s): return "".join(i for i in s if ord(i)<128)

def call_process(gv_command, map_string, raw_output = False):
	proc = Popen(gv_command, stdout=PIPE, stdin=PIPE, stderr=PIPE, shell=True)
	dot_out, map_err = proc.communicate(input = removeNonAscii(map_string))
	if map_err:
		print map_err
	if proc.returncode != 0:
		raise CallExternalException(map_err)
	if raw_output:
		return dot_out
	return removeNonAscii(dot_out)

def call_graphviz(task):
	try:
		return call_graphviz_int(task)
	except Exception as e:
		set_status(task, 'error<br>' + str(e))
		return None, None

def run_layout(task, layout_algorithm, map_string):
	if layout_algorithm == 'graph':
		return map_string

	set_status(task, 'running layout')
	return call_process(graphviz_command_layout(alg = layout_algorithm), map_string)

def run_clustering(task, cluster_algorithm, dot_out):
	if cluster_algorithm == 'graph':
		return dot_out
	if cluster_algorithm == 'cont-graph':
		set_status(task, 'making map contiguous')
		return call_process(ceba_command(), dot_out)

	set_status(task, 'running clustering')
	if cluster_algorithm == 'k-means':
		return call_process(cluster_command('graphkmeans'), dot_out)
	elif cluster_algorithm == 'cont-k-means':
		return call_process(cluster_command('geometrickmeans'), dot_out)
	elif cluster_algorithm == 'hierarchical':
		return call_process(cluster_command('graphhierarchical'), dot_out)
	elif cluster_algorithm == 'cont-hierarchical':
		return call_process(cluster_command('modularity-cont'), dot_out)
	elif cluster_algorithm == 'infomap':
		return call_process(cluster_command('infomap'), dot_out)
	elif cluster_algorithm == 'cont-infomap':
		dot_out = call_process(cluster_command('infomap'), dot_out)
		set_status(task, 'making map contiguous')
		return call_process(ceba_command(), dot_out)
	elif cluster_algorithm == 'modularity':
		return call_process(cluster_command('modularity'), dot_out)
	elif cluster_algorithm == 'cont-modularity':
		dot_out = call_process(cluster_command('modularity'), dot_out)
		set_status(task, 'making map contiguous')
		return call_process(ceba_command(), dot_out)

def run_color_assignment(task, dot_out):
	set_status(task, 'assigning colors')
	return call_process(colors_command(), dot_out)

def call_graphviz_int(task):
    map_string = task.input_dot
    vis_type = task.vis_type
    layout_algorithm = task.layout_algorithm
    cluster_algorithm = task.cluster_algorithm

    if vis_type == 'node-link':
    	#pipeline with no clustering
    	dot_out = run_layout(task, layout_algorithm, map_string)

    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out
    elif vis_type == 'gmap':
    	#default pipeline
    	dot_out = run_layout(task, layout_algorithm, map_string)
    	dot_out = run_clustering(task, cluster_algorithm, dot_out)
    	dot_out = call_process(graphviz_command_gmap(task.color_scheme), dot_out)
    	if task.color_scheme == 'bubble-sets':
    		dot_out = run_color_assignment(task, dot_out)

    	set_status(task, 'map construction')
    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out

    elif vis_type == 'point-cloud':
    	dot_out = run_layout(task, layout_algorithm, map_string)
    	dot_out = run_clustering(task, cluster_algorithm, dot_out)
    	dot_out = call_process(graphviz_command_gmap(task.color_scheme), dot_out)
    	if task.color_scheme == 'bubble-sets':
    		dot_out = run_color_assignment(task, dot_out)

    	set_status(task, 'point cloud construction')
    	dot_out = call_process(pointcloud_command(), dot_out)

    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out

    elif vis_type == 'bubble-sets':
    	dot_out = run_layout(task, layout_algorithm, map_string)
    	dot_out = run_clustering(task, cluster_algorithm, dot_out)
    	dot_out = call_process(graphviz_command_gmap(task.color_scheme), dot_out)
    	if task.color_scheme == 'bubble-sets':
    		dot_out = run_color_assignment(task, dot_out)

    	set_status(task, 'creating bubble sets')
    	dot_out = call_process(bubblesets_command(), dot_out)

    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out

    elif vis_type == 'line-sets':
    	dot_out = run_layout(task, layout_algorithm, map_string)
    	dot_out = run_clustering(task, cluster_algorithm, dot_out)
    	dot_out = call_process(graphviz_command_gmap(task.color_scheme), dot_out)
    	if task.color_scheme == 'bubble-sets':
    		dot_out = run_color_assignment(task, dot_out)

    	set_status(task, 'creating line sets')
    	dot_out = call_process(linesets_command(), dot_out)

    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out

    elif vis_type == 'map-sets':
    	dot_out = run_layout(task, layout_algorithm, map_string)
    	dot_out = run_clustering(task, cluster_algorithm, dot_out)
    	# running ceba
    	#if not cluster_algorithm.startswith('cont-'):
    	#	set_status(task, 'making map contiguous')
    	#	dot_out = call_process(ceba_command(), dot_out)

    	set_status(task, 'creating map sets')
    	#log.debug('MapSets-Input: %s' %(dot_out))
    	dot_out = call_process(mapsets_command(), dot_out)
    	dot_out = call_process(mapsets_post_command(task.color_scheme), dot_out)
    	if task.color_scheme == 'bubble-sets':
    		dot_out = run_color_assignment(task, dot_out)

    	svg_out = get_graphviz_map(dot_out, 'svg')
    	return dot_out, svg_out


def get_graphviz_map(map_string, file_format):
   	gv_command = graphviz_command_draw(file_format = file_format)
   	return call_process(gv_command, map_string, True)

def call_graphviz_scale(dot_rep, s1, s2):
   	gv_command = graphviz_command_scale(s1, s2)
   	return call_process(gv_command, dot_rep, True)

def set_status(task, s):
   	task.status = s
   	task.save()
    