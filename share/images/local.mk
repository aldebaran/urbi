## Copyright (C) 2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

imagesdir = $(brandsharedir)/images
gostai_jpegdir = $(imagesdir)/gostai-logo/jpeg
gostai_ppmdir = $(imagesdir)/gostai-logo/ppm
gostai_headerlessdir = $(imagesdir)/gostai-logo/hearderless
dist_gostai_jpeg_DATA := $(call ls_files,share/images/gostai-logo/jpeg/*)
dist_gostai_ppm_DATA := $(call ls_files,share/images/gostai-logo/ppm/*)
dist_gostai_headerless_DATA := $(call ls_files,share/images/gostai-logo/headerless/*)
