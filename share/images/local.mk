## Copyright (C) 2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

imagesdir = $(brandsharedir)/images
gostai_logodir = $(imagesdir)/gostai-logo
dist_gostai_logo_DATA := $(call ls_files,share/images/gostai-logo/*.jpg)
