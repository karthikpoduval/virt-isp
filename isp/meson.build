project('vx_isp', 'c')

# Define the sources for the C library
libisp_sources = files('vx_lib_isp.c', 'vx_demosaic.c')

# Define the sources for the C++ application
#bar_sources = files('bar.cpp', 'bar.h')

# Create a shared library target for the C library
libisp = shared_library('openvx-isp', libisp_sources, link_args : '-lopenvx')

# Create an executable target for the C++ application
#bar = executable('bar', bar_sources, cpp_args: cpp.get_supported_arguments('-std=c++11'), link_with: libisp)

