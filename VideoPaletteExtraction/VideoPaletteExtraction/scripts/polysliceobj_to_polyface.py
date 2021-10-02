import numpy as np
import sys

vertices = []
edges = []
faces = []
faces_a = []

if __name__ == '__main__':

    base_dir = sys.argv[1]
    output_name = sys.argv[2]
    obj_path =  base_dir + '/z_polyhedron_slices.obj'
    poly_path = base_dir + '/skew_polyhedron_palette/' + output_name

    # 4. 改这里导出文件的目录和文件名字
    n_frames = 0
    with open(obj_path, 'r') as f:
        for s in f:
            s = s.strip().split('\t')
            t, s = s[0], s[1:]
            if t == 'v':
                vertices.append(s)
            elif t == 'e':
                edges.append(s)
            elif t == 'fslice':
                if n_frames < int(s[0]):
                    n_frames = int(s[0])
                faces.append([s[0], s[1], s[3], s[4], s[6], s[7], s[9]])
                faces_a.append([s[2], s[5], s[8]])


    vertices = np.array(vertices).astype(np.float)
    edges = np.array(edges).astype(np.int32)
    faces = np.array(faces).astype(np.int32)
    faces_a = np.array(faces_a).astype(np.float)

    edges = edges - 1
    faces[:, 1:] = faces[:, 1:] - 1

    vertices = np.array(vertices).astype(np.float)
    edges = np.array(edges).astype(np.int32)
    faces = np.array(faces).astype(np.int32)
    faces_a = np.array(faces_a).astype(np.float)


    n_vertices = vertices.shape[0]
    n_edges = edges.shape[0]
    n_faces = faces.shape[0]

    '''
    print(faces[0])
    print(faces[-1])
    print(faces)
    print(faces_a)
    print(n_faces, faces.shape, faces_a.shape)
    '''

    # 5. 改这里输出的 poly 文件
    with open(poly_path, 'wb') as f:
        f.write(np.array([n_frames+1, n_vertices, n_edges, n_faces], dtype=np.int32).tobytes())

        f.write(vertices.tobytes())

        for p, q in edges:
            f.write(np.array([p, q], dtype=np.int32).tobytes())

        f.write(faces.tobytes())
        f.write(faces_a.tobytes())