env = Environment()
env.Append(CXXFLAGS=' -g')

prgTarget = SConscript('src/SConscript', variant_dir='build/', duplicate=0, exports='env')

env.Install('build/', 'src/shaders')
