bl_info = {
    "name": "Brechpunkt VBO exporter",
    "author": "IceCubeGames",
    "version": (0, 1),
    "blender": (2, 75, 0),
    "location": "File > Export",
    "description": "Brechpunkt VBO exporter",
    "warning": "",
    "wiki_url": "",
    "category": "Export",
}

import bpy
import json
from os import path
import struct
from mathutils import Vector, Euler
from math import sqrt, ceil, radians
import subprocess
import sys

def write_gi_block(context, filepath):
    directory, filename = path.split(filepath)

    with open(path.join(directory, bpy.data.groups[0].name + ".vbo"), "wb") as vbo:
        object = bpy.data.groups[0].objects[0]
        matrix = object.matrix_world
        
        mesh = object.to_mesh(bpy.context.scene, True, 'RENDER')
            
        vertices = mesh.vertices
        
        texture = mesh.tessface_uv_textures["UVMap"]

        for face_index in range(len(mesh.tessfaces)):
            face = mesh.tessfaces[face_index]
            
            texture_face = texture.data[face_index]
            texture_uvs = texture_face.uv
            
            vertex_indices = range(3)
            if len(face.vertices) == 4:
                vertex_indices = [0, 1, 2, 2, 3, 0]
            
            if len(face.vertices) in [3, 4]:
                for i in vertex_indices:
                    normal = vertices[face.vertices[i]].normal[:]
                    ray_cast_direction = -vertices[face.vertices[i]].normal
                    position = vertices[face.vertices[i]].co

                    hit, loc_hit, normal_hit, face_hit = object.ray_cast(
                        position + ray_cast_direction * 0.01, ray_cast_direction
                    )
                    
                    vbo.write(
                        struct.pack(
                            'fff', 
                            * (matrix * position)
                        ) + 
                        struct.pack(
                            'fff', * (matrix * Vector(
                                normal + (0,)
                            ))[:3]
                        ) +
                        struct.pack('ff', * texture_uvs[i]) +
                        struct.pack(
                            'fff',
                            * (matrix * loc_hit)
                        )
                    )
            else:
                print("Polygon is neither a triangle nor a square. Skipped.")

    return {'FINISHED'}

# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty
from bpy.types import Operator
   
class ExportBrechpunktObject(Operator, ExportHelper):
    """Export to Gi Block file"""
    # important since its how bpy.ops.import_test.some_data is constructed
    bl_idname = "brechpunkt.object_export"  
    bl_label = "Export Brechpunkt Object"

    # ExportHelper mixin class uses this
    filename_ext = ".vbo"

    filter_glob = StringProperty(
        default="*.vbo",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    def execute(self, context):
        return write_gi_block(context, self.filepath)

    def menu_func_export(self, context):
        self.layout.operator(bl_idname, text="Brechpunkt Object VBO (.vbo)")

def register():
    bpy.utils.register_class(ExportBrechpunktObject)
    bpy.types.INFO_MT_file_export.append(ExportBrechpunktObject.menu_func_export)

def unregister():
    unregisterSceneProperties()
    bpy.utils.unregister_class(ExportBrechpunktObject)
    bpy.types.INFO_MT_file_export.remove(ExportBrechpunktObject.menu_func_export)
