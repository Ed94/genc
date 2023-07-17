project( 'test', 'cpp', default_options : ['buildtype=debug'] )

includes = include_directories(
[
	'../bootstrap'
])

sources = [ '../bootstrap/genc.bootstrap.cpp' ]

if get_option('buildtype').startswith('debug')

  add_project_arguments('-DBuild_Debug', language : ['cpp'])

endif

add_project_arguments('-Dgen_time', language : ['cpp'])

executable( 'genc', sources, include_directories : includes )
