
import os
import sys	


def is_active():
	return True
        
def get_name():
        return "X11"


def can_build():

	if (os.name!="posix"):
		return False

	if sys.platform == "darwin":
		return False # no x11 on mac for now

	errorval=os.system("pkg-config --version > /dev/null")
	
	if (errorval):
		print("pkg-config not found.. x11 disabled.")
		return False
	
	x11_error=os.system("pkg-config x11 --modversion > /dev/null ")
	if (x11_error):
		print("X11 not found.. x11 disabled.")
		return False
		
		ssl_error=os.system("pkg-config openssl --modversion > /dev/null ")
		if (ssl_error):
			print("OpenSSL not found.. x11 disabled.")
			return False

	x11_error=os.system("pkg-config xcursor --modversion > /dev/null ")
	if (x11_error):
		print("xcursor not found.. x11 disabled.")
		return False

	
	return True # X11 enabled
  
def get_opts():

	return [
	('use_llvm','Use llvm compiler','no'),
	('use_sanitizer','Use llvm compiler sanitize address','no'),
	]
  
def get_flags():

	return [
	('builtin_zlib', 'no'),
	("builtin_openssl", "yes"),
	("theora","no"),
        ]
			


def configure(env):

	is64=sys.maxsize > 2**32

	if (env["bits"]=="default"):
		if (is64):
			env["bits"]="64"
		else:
			env["bits"]="32"


	env.Append(CPPPATH=['#platform/x11'])
	if (env["use_llvm"]=="yes"):
		env["CC"]="clang"
		env["CXX"]="clang++"
		env["LD"]="clang++"
		if (env["use_sanitizer"]=="yes"):
			env.Append(CXXFLAGS=['-fsanitize=address','-fno-omit-frame-pointer'])
			env.Append(LINKFLAGS=['-fsanitize=address'])
			env.extra_suffix=".llvms"
		else:
			env.extra_suffix=".llvm"




	#if (env["tools"]=="no"):
	#	#no tools suffix
	#	env['OBJSUFFIX'] = ".nt"+env['OBJSUFFIX']
	#	env['LIBSUFFIX'] = ".nt"+env['LIBSUFFIX']


	if (env["target"]=="release"):
		
		env.Append(CCFLAGS=['-O2','-ffast-math','-fomit-frame-pointer'])

	elif (env["target"]=="release_debug"):

		env.Append(CCFLAGS=['-O2','-ffast-math','-DDEBUG_ENABLED'])

	elif (env["target"]=="debug"):

		env.Append(CCFLAGS=['-g2', '-Wall','-DDEBUG_ENABLED','-DDEBUG_MEMORY_ENABLED'])

	env.ParseConfig('pkg-config x11 --cflags --libs')
	env.ParseConfig('pkg-config xcursor --cflags --libs')
	env.ParseConfig('pkg-config openssl --cflags --libs')


	env.ParseConfig('pkg-config freetype2 --cflags --libs')
	env.Append(CCFLAGS=['-DFREETYPE_ENABLED'])

	
	env.Append(CPPFLAGS=['-DOPENGL_ENABLED'])
	env.Append(CPPFLAGS=["-DALSA_ENABLED"])
	env.Append(CPPFLAGS=['-DX11_ENABLED','-DUNIX_ENABLED','-DGLES1_ENABLED', '-DGL_DLL_IMPORTS', '-DUSE_MGL_NAMESPACE'])
	env.Append(LIBS=['OSMesa32', 'GLU', 'SDL', 'pthread','asound','z']) #TODO detect linux/BSD!

#host compiler is default..

	if (is64 and env["bits"]=="32"):
		env.Append(CPPFLAGS=['-m32'])
		env.Append(LINKFLAGS=['-m32','-L/usr/lib/i386-linux-gnu'])
	elif (not is64 and env["bits"]=="64"):
		env.Append(CPPFLAGS=['-m64'])
		env.Append(LINKFLAGS=['-m64','-L/usr/lib/i686-linux-gnu'])


	if (env["CXX"]=="clang++"):
		env.Append(CPPFLAGS=['-DTYPED_METHOD_BIND'])
		env["CC"]="clang"
		env["LD"]="clang++"

	import methods

	env.Append( BUILDERS = { 'GLSL120' : env.Builder(action = methods.build_legacygl_headers, suffix = 'glsl.h',src_suffix = '.glsl') } )
	env.Append( BUILDERS = { 'GLSL' : env.Builder(action = methods.build_glsl_headers, suffix = 'glsl.h',src_suffix = '.glsl') } )
	env.Append( BUILDERS = { 'GLSL120GLES' : env.Builder(action = methods.build_gles2_headers, suffix = 'glsl.h',src_suffix = '.glsl') } )
	#env.Append( BUILDERS = { 'HLSL9' : env.Builder(action = methods.build_hlsl_dx9_headers, suffix = 'hlsl.h',src_suffix = '.hlsl') } )

