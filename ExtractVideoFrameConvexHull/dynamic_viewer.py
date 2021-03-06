#!/usr/bin/env python
import logging
import threading
import time
import sys
from GLUTWindow import GLUTWindow
from trimesh import TriMesh
import numpy as np

## Most of myarray.py:s
from numpy import *
def asarrayf( *args, **kwargs ):
    kwargs['dtype'] = float
    return asarray( *args, **kwargs )
def arrayf( *args, **kwargs ):
    kwargs['dtype'] = float
    return array( *args, **kwargs )
def mag2( vec ): return dot( vec, vec )
def mag( vec ): return sqrt( mag2( vec ) )
def direction( vec ): return vec * 1./mag(vec)

try:
    from OpenGL.GLUT import *
    from OpenGL.GL import *
    from OpenGL.GLU import *
except:
    raise ImportError('PyOpenGL not installed properly.')

class Camera( object ):
    def __init__( self ):
        self.center = arrayf( ( 0,0,0 ) )
        self.eye = arrayf( ( 0,0,2 ) )
        self.up = arrayf( ( 0,1,0 ) )
        self.near_far = asarrayf( ( .1, 4.1 ) )
        
        self.proj = self.Perspective( fovy = 50., aspect = 1. )
        #self.proj = self.Ortho( -1., 1., -1., 1. )
    
    class Perspective( object ):
        def __init__( self, **kwargs ):
            kwargs.setdefault( 'fovy', 40. )
            kwargs.setdefault( 'aspect', 1. )
            
            self.fovy = kwargs[ 'fovy' ]
            self.aspect = kwargs[ 'aspect' ]
        
        def apply( self, view_width, view_height, near, far ):
            gluPerspective(
                self.fovy, self.aspect * float( view_width ) / float( view_height ),
                near, far
                )
    
    class Ortho( object ):
        def __init__( self, left = -1., right = 1., bottom = -1., top = 1. ):
            self.left = left
            self.right = right
            self.bottom = bottom
            self.top = top
        
        def apply( self, view_width, view_height, near, far ):
            glOrtho( self.left, self.right, self.bottom, self.top, near, far )
    
    class Oblique( object ):
        def __init__( self, left = -1., right = 1., bottom = -1., top = 1., a = 0., b = 0., eye_distance_to_zero_plane = 0. ):
            self.left = left
            self.right = right
            self.bottom = bottom
            self.top = top
            self.a = a
            self.b = b
            self.z0 = eye_distance_to_zero_plane
        
        def apply( self, view_width, view_height, near, far ):
            glOrtho( self.left, self.right, self.bottom, self.top, near, far )
            shear_matrix = identity( 4 )
            shear_matrix[0,2] = self.a
            shear_matrix[1,2] = self.b
            #print 'shear_matrix:', shear_matrix
            glTranslated( 0, 0, -self.z0 )
            glMultMatrixd( shear_matrix.T.flatten() )
            glTranslated( 0, 0, self.z0 )
    
    def Orbit( self, axis, radians ):
        '''
        Orbits the camera about the view center using the rotation defined by 'axis' and 'radians'.
        '''
        
        self.eye = self.center + rotate( self.eye - self.center, axis, radians )
        self.up = rotate( self.up, axis, radians )
    
    def Rotate2( self, left_right, up_down ):
        '''
        Rotates the camera given image plane motion 'left_right' and 'up_down',
        scalars in the range -1..1.
        '''
        
        #print 'left_right:', left_right
        #print 'up_down:', up_down
        coef = pi
        
        self.Orbit( self.up, coef * left_right )
        self.Orbit( cross( self.up, self.eye - self.center ), coef * -up_down )
    
    def Apply( self ):
        self.__ApplyProjection()
        self.__ApplyModelview()
    
    def __ApplyProjection( self ):
        glMatrixMode( GL_PROJECTION )
        glLoadIdentity()
        
        view = glGetIntegerv( GL_VIEWPORT )
        view_width = view[2]
        view_height = view[3]
        del view
        
        self.proj.apply( view_width, view_height, self.near_far[0], self.near_far[1] )
    
    def __ApplyModelview( self ):
        glMatrixMode( GL_MODELVIEW )
        glLoadIdentity()
        
        gluLookAt(
            self.eye[0], self.eye[1], self.eye[2],
            self.center[0], self.center[1], self.center[2],
            self.up[0], self.up[1], self.up[2]
            )

def set_headlight( camera ):
    light_position = [ camera.eye[0], camera.eye[1], camera.eye[2], 1.0 ]
    glLightfv( GL_LIGHT0, GL_POSITION, light_position )
    glEnable( GL_LIGHT0 )

def draw_mesh_faces_flat( mesh, will_draw_edges = True ):
    '''
    Takes a TriMesh parameter mesh and draws it, setting as little OpenGL state as possible.
    Lighting and materials and colors be specified before this function is entered.
    If the parameters indicates that edges will be drawn, then glPolygonOffset will be used.
    '''
    
    if will_draw_edges:
        ## This offset guarantees that all polygon faces have a z-buffer value at least 1 greater.
        glPolygonOffset( 1.0, 1.0 )
        glEnable( GL_POLYGON_OFFSET_FILL )
    
    from itertools import izip
    glBegin( GL_TRIANGLES )
    for face, normal in izip( mesh.faces, mesh.face_normals ):
        glNormal3f( *normal )
        for vertex_index in face:
            glVertex3f( *mesh.vs[ vertex_index ] )
    glEnd()
    
    glDisable( GL_POLYGON_OFFSET_FILL )

dtype_type2glenum = {
    float: GL_DOUBLE,
    float32: GL_FLOAT,
    float64: GL_DOUBLE,
    int: GL_INT,
    int32: GL_INT,
    uint32: GL_UNSIGNED_INT
    }
def draw_mesh_faces_smooth( mesh, will_draw_edges = True ):
    '''
    Takes a TriMesh parameter mesh and draws it, setting as little OpenGL state as possible.
    Lighting and materials and colors be specified before this function is entered.
    If the parameters indicates that edges will be drawn, then glPolygonOffset will be used.
    '''
    
    ## I assume that mesh.vs[0] exists, so I need this.
    if len( mesh.vs ) == 0 or len( mesh.faces ) == 0: return
    assert len( mesh.vs ) == len( mesh.vertex_normals )
    
    if will_draw_edges:
        ## This offset guarantees that all polygon faces have a z-buffer value at least 1 greater.
        glPolygonOffset( 1.0, 1.0 )
        glEnable( GL_POLYGON_OFFSET_FILL )
    
    glEnableClientState( GL_VERTEX_ARRAY )
    ## Convert mesh.vs to an array() so that subsequent calls to this function are fast.
    mesh.vs = asarray( mesh.vs )
    mesh_vs = mesh.vs # asarray( mesh.vs )
    glVertexPointer( mesh_vs.shape[1], dtype_type2glenum[ mesh_vs.dtype.type ], 0, mesh_vs )
    ## NOTE: PyOpenGL's convenience glVertexPointerd(), glNormalPointerd(), and glDrawElementsui()
    ##       are painfully slow!  Why is that?  Why doesn't it just call asarray() and check the
    ##       resulting shape and dtype?
    #glVertexPointerd( mesh.vs )
    
    glEnableClientState( GL_NORMAL_ARRAY )
    ## mesh.vertex_normals is always an array, and we can't set the property to asarray() anyways.
    #mesh.vertex_normals = asarray( mesh.vertex_normals )
    mesh_normals = mesh.vertex_normals # asarray( mesh.vertex_normals )
    glNormalPointer( dtype_type2glenum[ mesh_normals.dtype.type ], 0, mesh_normals )
    #glNormalPointerd( mesh.vertex_normals )
    
    ## We can't do the dtype_type2glenum[] thing here because it must be an unsigned int.
    glDrawElements( GL_TRIANGLES, 3 * len( mesh.faces ), GL_UNSIGNED_INT, asarray( mesh.faces, dtype = uint32 ) )
    #glDrawElementsui( GL_TRIANGLES, mesh.faces )
    
    glDisableClientState( GL_NORMAL_ARRAY )
    glDisableClientState( GL_VERTEX_ARRAY )
    
    glDisable( GL_POLYGON_OFFSET_FILL )

## The default is flat shading.
draw_mesh_faces = draw_mesh_faces_flat

def draw_mesh_edges( mesh ):
    '''
    Takes a TriMesh parameter mesh and draws it, setting as little OpenGL state as possible.
    '''
    glShadeModel( GL_SMOOTH )
    glBegin( GL_LINES )
    for edge in mesh.edges:
        edge = tuple( edge )
        glColor(*(mesh.vs[ edge[0] ] + .5))
        glVertex3f( *mesh.vs[ edge[0] ] )
        glColor(*(mesh.vs[ edge[1] ] + .5))
        glVertex3f( *mesh.vs[ edge[1] ] )


    for p, q in [(0., -.5), (1., 0.5)]:
        for r, s in [(0., -.5), (1., 0.5)]:
            glColor(p, r, 0.)
            glVertex3f(q, s, -.5)
            glColor(p, r, 1.)
            glVertex3f(q, s, .5)
            glColor(p, 0., r)
            glVertex3f(q, -.5, s)
            glColor(p, 1., r)
            glVertex3f(q, .5, s)
            glColor(0., p, r)
            glVertex3f(-.5, q, s)
            glColor(1., p, r)
            glVertex3f(.5, q, s)
    glEnd()

def draw_linestrips( linestrips ):
    '''
    Takes a sequence of line strips, where each line strip is a sequence of points,
    and draws it, setting as little OpenGL state as possible.
    '''
    
    for linestrip in linestrips:
        glBegin( GL_LINE_STRIP )
        
        ## For Songrun
        
        glEnd()

def draw_points( mesh, points, video_points):
    '''
    Takes a sequence of points and draws it, setting as little OpenGL state as possible.
    '''
    
    glShadeModel( GL_SMOOTH )
    glBegin( GL_POINTS )
    

    # print(dir(mesh))
    for x, y, z in mesh.vs:
        # v = tuple( v )
        glColor(x + .5, y + .5, z + .5)
        glVertex3f(x, y, z)
        # print(v)

    glEnd()


    if video_points is not None:
        glPointSize( 2 )
        glBegin( GL_POINTS )

        idx = np.random.randint(video_points.shape[0], size=10000)

        for x, y, z in video_points[idx, :]:
            glColor(x, y, z)
            glVertex3f(x - .5, y - .5, z - .5)

        glEnd()


        glPointSize( 10 )
        
        glBegin( GL_POINTS )
        glColor(1., 1., 1.)
        # glVertex3f(0.1722773 - .5, 0.93885602- .5, 0.84093276- .5)
        # glVertex3f(0.30301679 - .5, 0.94    - .5   , 0.94    - .5)

        #dzj remove the white vertex: glVertex3f(0.44313725 - .5 , 0.94       - .5 , 0.36627451 - .5 )

        glEnd()

    ## For Songrun
    

class TriMeshWindow( GLUTWindow ):
    def __init__( self, **kwargs ):
        GLUTWindow.__init__( self )
        
        kwargs.setdefault( 'background_color', ( .3, .3, .3 ) )
        kwargs.setdefault( 'mesh', TriMesh() )
        kwargs.setdefault( 'linestrips', [] )
        kwargs.setdefault( 'points', [] )
        kwargs.setdefault( 'camera', Camera() )
        kwargs.setdefault( 'draw_faces', True )
        kwargs.setdefault( 'draw_edges', True )
        kwargs.setdefault( 'draw_linestrips', True )
        kwargs.setdefault( 'draw_points', True )
        
        self.background_color = kwargs[ 'background_color' ]
        self.mesh = kwargs[ 'mesh' ]
        self.linestrips = kwargs[ 'linestrips' ]
        self.points = kwargs[ 'points' ]
        self.camera = kwargs[ 'camera' ]
        self.draw_faces = kwargs[ 'draw_faces' ]
        self.draw_edges = kwargs[ 'draw_edges' ]
        self.draw_linestrips = kwargs[ 'draw_linestrips' ]
        self.draw_points = kwargs[ 'draw_points' ]
        
        ## I don't want dynamic binding here because
        ## subclasses' constructors haven't run yet
        TriMeshWindow.reset( self )

        self.startPeriodicTimer( 100 )

    def refresh_time(self):
        
        # mesh_file_name = '%05d-convexhull_vertices-mesh.obj' % self.mesh_id
        
        mesh_file_name = self.prefix + '_%05d.obj' % self.mesh_id
       
        mesh = TriMesh.FromOBJ_FileName( self.mesh_path + mesh_file_name)
        
        # mesh = TriMesh.FromOBJ_FileName( self.mesh_path + '%05d-convexhull_vertices-mesh.obj' % self.mesh_id)


        self.setWindowTitle('%05d' % self.mesh_id + " " + mesh_file_name)
        
        mesh.vs = asarray( mesh.vs, dtype = float)
        
        mesh.vs -= .5
        mesh.positions_changed()
        self.mesh = mesh
        
        self.video_points = np.load(self.mesh_path + '%05d.img.npz.npy' % self.mesh_id).reshape(-1, 3) / 255.
        #print(self.video_points)

        self.postRedisplay()
        
    def timerFunc( self ):
        if self.play == False: return
        # print('time')
        ## This function would never return if we didn't fork().
        # import os
        # pid = os.fork()
        # if pid != 0: return pid
        self.mesh_id += 1
        self.refresh_time()
    
    def reset( self ):
        print('Resetting')
        
        self.postRedisplay()
    
    def displayFunc( self ):
        
        col = self.background_color
        glClearColor( col[0], col[1], col[2], 1.0 )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        glEnable( GL_DEPTH_TEST )
        
        self.camera.Apply()
        ## TODO: Something with the light.  Right now, we're getting OpenGL's default light.
        
        set_headlight( self.camera )
        glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE )
        glShadeModel( GL_SMOOTH )
        
        if self.draw_faces:
            glEnable( GL_LIGHTING )
            
            glEnable( GL_COLOR_MATERIAL )
            glColor3ub( 200, 200, 255 )
            # draw_mesh_faces( self.mesh, self.draw_edges or self.draw_linestrips or self.draw_points )
        
        if self.draw_edges:
            glDisable( GL_LIGHTING )
            
            glLineWidth( 1 )
            glColor3ub( 0,0,0 )
            draw_mesh_edges( self.mesh )
        
        if self.draw_linestrips:
            glLineWidth( 3 )
            draw_linestrips( self.linestrips )
        
        if self.draw_points:
            glPointSize( 10 )
            draw_points( self.mesh, self.points, self.video_points )
    
    def motionFunc( self, x, y ):
        ## Very simply map the camera onto +z half of the sphere x^2 + y^2 + z^2 = 1
        
        view = glGetIntegerv( GL_VIEWPORT )
        assert view[0] == 0
        assert view[1] == 0
        
        wx = view[2]/2.
        wy = view[3]/2.
        
        ## Flip y so that 0,0 is in the bottom-left.
        y = view[3] - y - 1
        
        radius = min( view[2], view[3] )/2.
        sphere_x = ( x - wx ) / radius
        sphere_y = ( y - wy ) / radius
        
        ## Project onto the closest point on the sphere
        if sphere_x*sphere_x + sphere_y*sphere_y > 1:
            denom = sqrt( sphere_x*sphere_x + sphere_y*sphere_y )
            sphere_x /= denom
            sphere_y /= denom
        
        sphere_z = sqrt( max( 0, 1 - sphere_x*sphere_x - sphere_y*sphere_y ) )
        
        eye_mag = mag( self.camera.eye )
        self.camera.eye = eye_mag * direction( arrayf( ( sphere_x, sphere_y, sphere_z ) ) )
        # print 'New eye position: ', self.camera.eye
        
        glutPostRedisplay()
    
    def mouseFunc( self, button, state, x,y ):
        # if state == GLUT_DOWN: pass
        pass
    
    def passiveMotionFunc( self, x,y ):
        pass
    
    def keyboardFunc( self, key, x, y ):
        if key == 'd' or key == 'D':
            self.mesh_id += 1
            self.refresh_time()
        if key == 'a' or key == 'A':
            self.mesh_id -= 1
            self.refresh_time()



        if 'r' == key: self.reset()
        elif 'w' == key: self.draw_edges = not self.draw_edges
        else: GLUTWindow.keyboardFunc( self, key, x, y )
        if key == 'q' or key == 'Q': sys.exit(0)

        '''
        if key == 'c': self.captureScreen()
        if key == '\\': self.toggleFullScreen()
        '''
        
        glutPostRedisplay()

def view_mesh( mesh_path,prefix, title = None):
    '''
    Given a TriMesh object 'mesh' and
    optional window title 'title',
    visualizes the mesh in a window by
    spawning a separate process.
    Returns immediately, with the visualization process ID
    as the return value.
    '''
    
    # ## This function would never return if we didn't fork().
    # # import os
    # # pid = os.fork()
    # # if pid != 0: return pid
    # mesh = TriMesh.FromOBJ_FileName( mesh_path + "%05d-convexhull_vertices-mesh.obj" % 1)

    # ## center and normalize the mesh
    # mesh.vs = asarray( mesh.vs, dtype = float )
    # ## Move the midpoint to the origin
    # # vsmax = mesh.vs.max( axis = 0 )
    # # vsmin = mesh.vs.min( axis = 0 )
    # # mesh.vs -= .5*( vsmax + vsmin )
    # ## Divide by the corner
    # # mesh.vs *= 1./( ( vsmax - vsmin ).max() )
    # ## Positions have changed, but not topology:
    # # mesh.vs /= 255.
    # mesh.vs -= .5
    # print(mesh.vs.min( axis = 0 ))
    # print(mesh.vs.max( axis = 0 ))
    # mesh.positions_changed()

    
    if title is None: title = ''
    
    glutInit( sys.argv )
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH )
    
    w1 = TriMeshWindow()
    # w1.setWindowTitle( title )
    # w1.mesh = mesh
    w1.mesh_path = mesh_path
    w1.prefix = prefix
    w1.mesh_id = 0
    w1.video_points = None
    w1.play = False
    w1.refresh_time()

    
    #w2 = TriMeshWindow()
    #w2pos = w2.getWindowPosition()
    #w2pos = ( w2pos[0] + w1.getWindowShape()[0], w2pos[1] )
    w1.positionWindow( [500, 0] if 'sim' in mesh_path else [0, 0] )
    
    glutMainLoop()

def view_lines( lines, title = None ):
    raise NotImplementedError( 'Songrun, you can implement this.' )

def main():
    if len( sys.argv ) not in [2, 3]:
        print('Usage:', sys.argv[0], 'mesh.obj')
        sys.exit(-1)
    
    mesh_path = sys.argv[1]
    prefix = sys.argv[2]
    print(mesh_path)
    pid = view_mesh( mesh_path, prefix,mesh_path )
    # print('Background process id:', pid)

    x = threading.Thread(target=view_mesh, args=(mesh_path,prefix, mesh_path))
    x.start()

    # if(sys.argv == 3):
    #     y = threading.Thread(target=view_mesh, args=(sys.argv[2], sys.argv[2]))
    #     y.start()
    #     y.join()

    # x.join()




# call main() if the script is being run
if __name__ == '__main__': main()
