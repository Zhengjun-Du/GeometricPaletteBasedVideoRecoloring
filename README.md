## Video Recoloring via Spatial-Temporal Geometric Palettes

![](E:\video_palette_project\teaser.png)

This is the source code of the paper: **Video Recoloring via Spatial-Temporal Geometric Palettes**, authors: Zheng-Jun Du, Kai-Xiang Lei, Kun Xu, Jianchao Tan, Yotam Gingold. Please fell free to contact us if you have any questions, email: duzj19@mails.tsinghua.edu.cn

### Requirements

Windows 10,  Microsoft Visual Studio 2015,  Python 3.7,  Qt5.14.1(for Video recoloring GUI)

### Directories

1. data:  the video "season" that shown in our paper
2. VideoPaletteExtraction: the video palette extraction program
3. RecolorGUI: the video recoloring GUI

### Usage

1. Generate single frame's convex hull with Jianchao Tan's method: https://github.com/JianchaoTan/fastLayerDecomposition. Here I provide the frames' convex hulls in "data/season/" , user need not to generated it again.

2. Run the VideoPaletteExtraction program, the generated video palette will save in "data/season/skew_polyhedron_palette"

3. Run "data/scripts/images_to_video_file.py" to generate the original uncompressed video, which will save in  "data/season/skew_polyhedron_palette";

4. Run the RecolorGUI to recolor the video:

   a) click "Open Video and Palette" to load the video and palette, which locate in "data/season/skew_polyhedron_palette"

   b) click "Calculate weights" to calculate the MVC weights

   c)  change the palette colors in the "Palette Timeline" view

![](E:\video_palette_project\recolor-ui.png)

### References

[1] Du Z J, Lei K X, Xu K, et al. Video recoloring via spatial-temporal geometric palettes[J]. ACM Transactions on Graphics (TOG), 2021, 40(4): 1-16.

[2] Wang Y, Liu Y, Xu K. An Improved Geometric Approach for Palette‐based Image Decomposition and Recoloring[C]//Computer Graphics Forum. 2019, 38(7): 11-22.

[3] Tan J, Lien J M, Gingold Y. Decomposing images into layers via RGB-space geometry[J]. ACM Transactions on Graphics (TOG), 2016, 36(1): 1-14.