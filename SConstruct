
import os

############################################################# [ World options ]
Progress('$TARGET\r', overwrite=True, file=open('con', 'w'))

# Auto select the number of processors
if os.path.exists('/proc'):
  cpus = len([l for l in open('/proc/cpuinfo') if l.startswith('processor\t')])
else:
  cpus = 1
SetOption('num_jobs', cpus + 1)

SConscript("SConscript", build_dir="build", duplicate=0)
