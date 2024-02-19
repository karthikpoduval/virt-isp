project('vx_test', 'c')
glib_dep = dependency('glib-2.0')
#openvx_dep = declare_dependency(link_args : ['-lopenvx']) 
#m_dep = dependency('m', method : 'build-in')

#executable('vx_test', 'vx_test.c', dependencies : [openvx_dep, glib_dep, m_dep])
executable('vx_test', 'vx_test.c', dependencies : [glib_dep], link_args : '-lopenvx')
