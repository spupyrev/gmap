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

    def metadata(self):
        return {
            'id': self.id,
            'status': self.status,
            'width': self.width,
            'height': self.height,
            'semantic_zoom': self.semantic_zoom,
        }

    def json_metadata(self):
        return dumps(self.metadata())
        