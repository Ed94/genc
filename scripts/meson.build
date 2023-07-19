project( 'genc', 'cpp', default_options : ['buildtype=debug'] )

cc = meson.get_compiler('cpp')

includes = include_directories(
[
	'../bootstrap'
])

sources = [ '../bootstrap/genc.bootstrap.cpp' ]

if get_option('buildtype').startswith('debug')

  add_project_arguments('-DBuild_Debug', language : ['cpp'])

endif

add_project_arguments('-DGEN_TIME', language : ['cpp'])

# Necessary for gcc
if cc.get_id() == 'gcc'
  add_project_arguments('-Wno-unknown-pragmas', language: 'cpp')
endif

executable( 'genc', sources, include_directories : includes )
