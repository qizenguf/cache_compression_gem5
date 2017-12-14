import m5
from m5.objects import *

# These three directory paths are not currently used.
#GEM5_DIR=/home/group/Documents/gem5                          # Install location of gem5
#SPEC_DIR=/home/group/cpu2006                 # Install location of your SPEC2006 benchmarks
#out_dir = '<FULL_PATH_TO_DESIRED_OUTPUT_DIRECTORY>'

alpha_suffix = '_base.my-alpha'

#temp
#binary_dir = spec_dir
#data_dir = spec_dir

#400.perlbench
perlbench = LiveProcess()
perlbench.executable =  'perlbench' + alpha_suffix
# TEST CMDS
#perlbench.cmd = [perlbench.executable] + ['-I.', '-I./lib', 'attrs.pl']
# REF CMDS
perlbench.cmd = [perlbench.executable] + ['-I./lib', 'checkspam.pl', '2500', '5', '25', '11', '150', '1', '1', '1', '1']
#perlbench.cmd = [perlbench.executable] + ['-I./lib', 'diffmail.pl', '4', '800', '10', '17', '19', '300']
#perlbench.cmd = [perlbench.executable] + ['-I./lib', 'splitmail.pl', '1600', '12', '26', '16', '4500']
#perlbench.output = out_dir+'perlbench.out'

#401.bzip2
bzip2 = LiveProcess()
bzip2.executable =  '/home/group/cpu2006/benchspec/CPU2006/401.bzip2/run/run_base_ref_my-alpha.0000/bzip2' + alpha_suffix
# TEST CMDS
#bzip2.cmd = [bzip2.executable] + ['input.program', '5']
# REF CMDS
bzip2.cmd = [bzip2.executable] + ['/home/group/cpu2006/benchspec/CPU2006/401.bzip2/run/run_base_ref_my-alpha.0000/input.source', '280']
#bzip2.cmd = [bzip2.executable] + ['chicken.jpg', '30']
#bzip2.cmd = [bzip2.executable] + ['liberty.jpg', '30']
#bzip2.cmd = [bzip2.executable] + ['input.program', '280']
#bzip2.cmd = [bzip2.executable] + ['text.html', '280']
#bzip2.cmd = [bzip2.executable] + ['input.combined', '200']
#bzip2.output = out_dir + 'bzip2.out'

#403.gcc
gcc = LiveProcess()
gcc.executable = '/home/group/cpu2006/benchspec/CPU2006/403.gcc/run/run_base_ref_my-alpha.0000/gcc' + alpha_suffix
# TEST CMDS
#gcc.cmd = [gcc.executable] + ['cccp.i', '-o', 'cccp.s']
# REF CMDS
gcc.cmd = [gcc.executable] + ['/home/group/cpu2006/benchspec/CPU2006/403.gcc/run/run_base_ref_my-alpha.0000/166.i', '-o', '/home/group/cpu2006/benchspec/CPU2006/403.gcc/run/run_base_ref_my-alpha.0000/166.s']
#gcc.cmd = [gcc.executable] + ['200.i', '-o', '200.s']
#gcc.cmd = [gcc.executable] + ['c-typeck.i', '-o', 'c-typeck.s']
#gcc.cmd = [gcc.executable] + ['cp-decl.i', '-o', 'cp-decl.s']
#gcc.cmd = [gcc.executable] + ['expr.i', '-o', 'expr.s']
#gcc.cmd = [gcc.executable] + ['expr2.i', '-o', 'expr2.s']
#gcc.cmd = [gcc.executable] + ['g23.i', '-o', 'g23.s']
#gcc.cmd = [gcc.executable] + ['s04.i', '-o', 's04.s']
#gcc.cmd = [gcc.executable] + ['scilab.i', '-o', 'scilab.s']
#gcc.output = out_dir + 'gcc.out'

#410.bwaves
bwaves = LiveProcess()
bwaves.executable = 'bwaves' + alpha_suffix
# TEST CMDS
#bwaves.cmd = [bwaves.executable]
# REF CMDS
bwaves.cmd = [bwaves.executable]
#bwaves.output = out_dir + 'bwaves.out'

#416.gamess
gamess=LiveProcess()
gamess.executable = 'gamess' + alpha_suffix
# TEST CMDS
#gamess.cmd = [gamess.executable]
#gamess.input = 'exam29.config'
# REF CMDS
gamess.cmd = [gamess.executable]
gamess.input = 'cytosine.2.config'
#gamess.cmd = [gamess.executable]
#gamess.input = 'h2ocu2+.gradient.config'
#gamess.cmd = [gamess.executable]
#gamess.input = 'triazolium.config'
#gamess.output = out_dir + 'gamess.out'

#429.mcf
mcf = LiveProcess()
mcf.executable =  'mcf' + alpha_suffix
# TEST CMDS
#mcf.cmd = [mcf.executable] + ['inp.in']
# REF CMDS
mcf.cmd = [mcf.executable] + ['inp.in']
#mcf.output = out_dir + 'mcf.out'

#433.milc
milc=LiveProcess()
milc.executable = 'milc' + alpha_suffix
# TEST CMDS
#milc.cmd = [milc.executable]
#milc.input = 'su3imp.in'
# REF CMDS
milc.cmd = [milc.executable]
milc.input = 'su3imp.in'
#milc.output = out_dir + 'milc.out'

#434.zeusmp
zeusmp=LiveProcess()
zeusmp.executable = 'zeusmp' + alpha_suffix
# TEST CMDS
#zeusmp.cmd = [zeusmp.executable]
# REF CMDS
zeusmp.cmd = [zeusmp.executable]
#zeusmp.output = out_dir + 'zeusmp.out'

#435.gromacs
gromacs = LiveProcess()
gromacs.executable = 'gromacs' + alpha_suffix
# TEST CMDS
#gromacs.cmd = [gromacs.executable] + ['-silent','-deffnm', 'gromacs', '-nice','0']
# REF CMDS
gromacs.cmd = [gromacs.executable] + ['-silent','-deffnm', 'gromacs', '-nice','0']
#gromacs.output = out_dir + 'gromacs.out'

#436.cactusADM
cactusADM = LiveProcess()
cactusADM.executable = 'cactusADM' + alpha_suffix 
# TEST CMDS
#cactusADM.cmd = [cactusADM.executable] + ['benchADM.par']
# REF CMDS
cactusADM.cmd = [cactusADM.executable] + ['benchADM.par']
#cactusADM.output = out_dir + 'cactusADM.out'

#437.leslie3d
leslie3d=LiveProcess()
leslie3d.executable = 'leslie3d' + alpha_suffix
# TEST CMDS
#leslie3d.cmd = [leslie3d.executable]
#leslie3d.input = 'leslie3d.in'
# REF CMDS
leslie3d.cmd = [leslie3d.executable]
leslie3d.input = 'leslie3d.in'
#leslie3d.output = out_dir + 'leslie3d.out'

#444.namd
namd = LiveProcess()
namd.executable = 'namd' + alpha_suffix
# TEST CMDS
#namd.cmd = [namd.executable] + ['--input', 'namd.input', '--output', 'namd.out', '--iterations', '1']
# REF CMDS
namd.cmd = [namd.executable] + ['--input', 'namd.input', '--output', 'namd.out', '--iterations', '38']
#namd.output = out_dir + 'namd.out'

#445.gobmk
gobmk=LiveProcess()
gobmk.executable = 'gobmk' + alpha_suffix
# TEST CMDS
#gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
#gobmk.input = 'dniwog.tst'
# REF CMDS
gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
gobmk.input = '13x13.tst'
#gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
#gobmk.input = 'nngs.tst'
#gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
#gobmk.input = 'score2.tst'
#gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
#gobmk.input = 'trevorc.tst'
#gobmk.cmd = [gobmk.executable] + ['--quiet','--mode', 'gtp']
#gobmk.input = 'trevord.tst'
#gobmk.output = out_dir + 'gobmk.out'

#447.dealII
####### NOT WORKING #########
dealII=LiveProcess()
dealII.executable = 'dealII' + alpha_suffix
# TEST CMDS
####### NOT WORKING #########
#dealII.cmd = [gobmk.executable]+['8']
# REF CMDS
####### NOT WORKING #########
#dealII.output = out_dir + 'dealII.out'

#450.soplex
soplex=LiveProcess()
soplex.executable = 'soplex' + alpha_suffix
# TEST CMDS
#soplex.cmd = [soplex.executable] + ['-m10000', 'test.mps']
# REF CMDS
soplex.cmd = [soplex.executable] + ['-m45000', 'pds-50.mps']
#soplex.cmd = [soplex.executable] + ['-m3500', 'ref.mps']
#soplex.output = out_dir + 'soplex.out'

#453.povray
povray=LiveProcess()
povray.executable = 'povray' + alpha_suffix
# TEST CMDS
#povray.cmd = [povray.executable] + ['SPEC-benchmark-test.ini']
# REF CMDS
povray.cmd = [povray.executable] + ['SPEC-benchmark-ref.ini']
#povray.output = out_dir + 'povray.out'

#454.calculix
calculix=LiveProcess()
calculix.executable = 'calculix' + alpha_suffix
# TEST CMDS
#calculix.cmd = [calculix.executable] + ['-i', 'beampic']
# REF CMDS
calculix.cmd = [calculix.executable] + ['-i', 'hyperviscoplastic']
#calculix.output = out_dir + 'calculix.out' 

#456.hmmer
hmmer=LiveProcess()
hmmer.executable = 'hmmer' + alpha_suffix
# TEST CMDS
#hmmer.cmd = [hmmer.executable] + ['--fixed', '0', '--mean', '325', '--num', '45000', '--sd', '200', '--seed', '0', 'bombesin.hmm']
# REF CMDS
hmmer.cmd = [hmmer.executable] + ['nph3.hmm', 'swiss41']
#hmmer.cmd = [hmmer.executable] + ['--fixed', '0', '--mean', '500', '--num', '500000', '--sd', '350', '--seed', '0', 'retro.hmm']
#hmmer.output = out_dir + 'hmmer.out'

#458.sjeng
sjeng=LiveProcess()
sjeng.executable = 'sjeng' + alpha_suffix 
# TEST CMDS
#sjeng.cmd = [sjeng.executable] + ['test.txt']
# REF CMDS
sjeng.cmd = [sjeng.executable] + ['ref.txt']
#sjeng.output = out_dir + 'sjeng.out'

#459.GemsFDTD
GemsFDTD=LiveProcess()
GemsFDTD.executable = 'GemsFDTD' + alpha_suffix 
# TEST CMDS
#GemsFDTD.cmd = [GemsFDTD.executable]
# REF CMDS
GemsFDTD.cmd = [GemsFDTD.executable]
#GemsFDTD.output = out_dir + 'GemsFDTD.out'

#462.libquantum
libquantum=LiveProcess()
libquantum.executable = 'libquantum' + alpha_suffix
# TEST CMDS
#libquantum.cmd = [libquantum.executable] + ['33','5']
# REF CMDS
libquantum.cmd = [libquantum.executable] + ['1297','8']
#libquantum.output = out_dir + 'libquantum.out' 

#464.h264ref
h264ref=LiveProcess()
h264ref.executable = 'h264ref' + alpha_suffix
# TEST CMDS
#h264ref.cmd = [h264ref.executable] + ['-d', 'foreman_test_encoder_baseline.cfg']
# REF CMDS
h264ref.cmd = [h264ref.executable] + ['-d', 'foreman_ref_encoder_baseline.cfg']
#h264ref.cmd = [h264ref.executable] + ['-d', 'foreman_ref_encoder_main.cfg']
#h264ref.cmd = [h264ref.executable] + ['-d', 'sss_encoder_main.cfg']
#h264ref.output = out_dir + 'h264ref.out'

#465.tonto
tonto=LiveProcess()
tonto.executable = 'tonto' + alpha_suffix
# TEST CMDS
#tonto.cmd = [tonto.executable]
# REF CMDS
tonto.cmd = [tonto.executable]
#tonto.output = out_dir + 'tonto.out'

#470.lbm
lbm=LiveProcess()
lbm.executable = 'lbm' + alpha_suffix
# TEST CMDS
#lbm.cmd = [lbm.executable] + ['20', 'reference.dat', '0', '1', '100_100_130_cf_a.of']
# REF CMDS
lbm.cmd = [lbm.executable] + ['300', 'reference.dat', '0', '0', '100_100_130_ldc.of']
#lbm.output = out_dir + 'lbm.out'

#471.omnetpp
omnetpp=LiveProcess()
omnetpp.executable = 'omnetpp' + alpha_suffix 
# TEST CMDS
#omnetpp.cmd = [omnetpp.executable] + ['omnetpp.ini']
# REF CMDS
omnetpp.cmd = [omnetpp.executable] + ['omnetpp.ini']
#omnetpp.output = out_dir + 'omnetpp.out'

#473.astar
astar=LiveProcess()
astar.executable = 'astar' + alpha_suffix
# TEST CMDS
#astar.cmd = [astar.executable] + ['lake.cfg']
# REF CMDS
astar.cmd = [astar.executable] + ['rivers.cfg']
#astar.output = out_dir + 'astar.out'

#481.wrf
wrf=LiveProcess()
wrf.executable = 'wrf' + alpha_suffix
# TEST CMDS
#wrf.cmd = [wrf.executable]
# REF CMDS
wrf.cmd = [wrf.executable]
#wrf.output = out_dir + 'wrf.out'

#482.sphinx3
sphinx3=LiveProcess()
sphinx3.executable = 'sphinx_livepretend' + alpha_suffix 
# TEST CMDS
#sphinx3.cmd = [sphinx3.executable] + ['ctlfile', '.', 'args.an4']
# REF CMDS
sphinx3.cmd = [sphinx3.executable] + ['ctlfile', '.', 'args.an4']
#sphinx3.output = out_dir + 'sphinx3.out'

#483.xalancbmk
######## NOT WORKING ###########
xalancbmk=LiveProcess()
xalancbmk.executable = 'xalancbmk' + alpha_suffix
# TEST CMDS
######## NOT WORKING ###########
#xalancbmk.cmd = [xalancbmk.executable] + ['-v','test.xml','xalanc.xsl']
# REF CMDS
######## NOT WORKING ###########
#xalancbmk.output = out_dir + 'xalancbmk.out'

#998.specrand
specrand_i=LiveProcess()
specrand_i.executable = 'specrand' + alpha_suffix
# TEST CMDS
#specrand_i.cmd = [specrand_i.executable] + ['324342', '24239']
# REF CMDS
specrand_i.cmd = [specrand_i.executable] + ['1255432124', '234923']
#specrand_i.output = out_dir + 'specrand_i.out'

#999.specrand
specrand_f=LiveProcess()
specrand_f.executable = 'specrand' + alpha_suffix
# TEST CMDS
#specrand_f.cmd = [specrand_f.executable] + ['324342', '24239']
# REF CMDS
specrand_f.cmd = [specrand_f.executable] + ['1255432124', '234923']
#specrand_f.output = out_dir + 'specrand_f.out'
