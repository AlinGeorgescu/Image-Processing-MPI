# Image-Processing-MPI
Distributed Image Processing using Open MPI

The program edits PNM and PGM files, applying some image filters:
* Smoothing
* Approximative Gaussian Blur
* Sharpen
* Mean removal
* Emboss

How to run: 
* mpirun -np N ./tema3 image_in.pnm image_out.pnm filter1 filter2 ... filterX
or
* make run TASKS=$(num_tasks) IN=$(file_in) OUT=$(file_out) FILTERS=$(filters)
