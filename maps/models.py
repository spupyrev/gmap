from django.db import models
from json import dumps

class Task(models.Model):
    creation_date = models.CharField(null=True, max_length=64)
    creation_ip = models.CharField(null=True, max_length=64)

    input_dot = models.TextField()

    dot_rep = models.TextField()
    svg_rep = models.TextField()

    svg_rep0 = models.TextField(null=True)
    svg_rep1 = models.TextField(null=True)
    svg_rep2 = models.TextField(null=True)
    svg_rep3 = models.TextField(null=True)

    status = models.TextField()

    width = models.FloatField(null=True, blank=True)
    height = models.FloatField(null=True, blank=True)

    vis_type = models.CharField(max_length=64)
    layout_algorithm = models.CharField(max_length=64)
    cluster_algorithm = models.CharField(max_length=64)
    contiguous_algorithm = models.CharField(max_length=64)

    color_scheme  = models.CharField(max_length=64)
    semantic_zoom  = models.CharField(max_length=64)
    spherical = models.CharField(max_length=64)

    def metadata(self):
        return {
            'id': self.id,
            'status': self.status,
            'width': self.width,
            'height': self.height,
            'semantic_zoom': self.semantic_zoom,
            'spherical': self.spherical
        }

    def json_metadata(self):
        return dumps(self.metadata())

    def description(self):
        desc = ''
        desc += 'Visualization Type: ' + self.vis_type + '\n'
        desc += 'Layout Algorithm: ' + self.layout_algorithm + '\n'
        desc += 'Cluster Algorithm: ' + self.cluster_algorithm + '\n'
        desc += 'Color Scheme: ' + self.color_scheme + '\n'
        desc += 'Semantic Zoom: ' + self.semantic_zoom + '\n'
        desc += 'Spherical" ' + self.spherical + '\n'
        return desc

        