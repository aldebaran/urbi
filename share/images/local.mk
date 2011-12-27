## Copyright (C) 2010, 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

imagesdir = $(brandsharedir)/images
dist_images_DATA = share/images/README.txt

jpegdir = $(imagesdir)/gostai-logo/jpeg
dist_jpeg_DATA := $(call ls_files, share/images/gostai-logo/jpeg/*)

sourcesdir = $(imagesdir)/gostai-logo/sources
dist_sources_DATA := $(call ls_files, share/images/gostai-logo/sources/*)

converteddir = $(imagesdir)/gostai-logo/converted
dist_converted_DATA := $(call ls_files, share/images/gostai-logo/converted/*)
