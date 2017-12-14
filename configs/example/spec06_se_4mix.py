import spec06_benchmarks
import optparse
import sys
import os

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import addToPath, fatal

addToPath('../common')
addToPath('../ruby')
addToPath('../topologies')

import Options
import Ruby
import Simulation
import CacheConfig_l3
import MemConfig
from Caches_l3 import *
from cpu2000 import *

def get_processes(options):
    """Interprets provided options and returns a list of processes"""

    multiprocesses = []
    inputs = []
    outputs = []
    errouts = []
    pargs = []

    workloads = options.cmd.split(';')
    if options.input != "":
        inputs = options.input.split(';')
    if options.output != "":
        outputs = options.output.split(';')
    if options.errout != "":
        errouts = options.errout.split(';')
    if options.options != "":
        pargs = options.options.split(';')

    idx = 0
    for wrkld in workloads:
        process = LiveProcess()
        process.executable = wrkld
        process.cwd = os.getcwd()

        if len(pargs) > idx:
            process.cmd = [wrkld] + pargs[idx].split()
        else:
            process.cmd = [wrkld]

        if len(inputs) > idx:
            process.input = inputs[idx]
        if len(outputs) > idx:
            process.output = outputs[idx]
        if len(errouts) > idx:
            process.errout = errouts[idx]

        multiprocesses.append(process)
        idx += 1

    if options.smt:
        assert(options.cpu_type == "detailed" or options.cpu_type == "inorder")
        return multiprocesses, idx
    else:
        return multiprocesses, 1


parser = optparse.OptionParser()
Options.addCommonOptions(parser)
Options.addSEOptions(parser)

parser.add_option("-b", "--benchmark", type="string", default="", help="The SPEC benchmark to be loaded.")
parser.add_option("--benchmark_stdout", type="string", default="", help="Absolute path for stdout redirection for the benchmark.")
parser.add_option("--benchmark_stderr", type="string", default="", help="Absolute path for stderr redirection for the benchmark.")

if '--ruby' in sys.argv:
    Ruby.define_options(parser)

(options, args) = parser.parse_args()

if args:
    print "Error: script doesn't take any positional arguments"
    sys.exit(1)

# default LLC(L3) partition 0, we should specify this args. Otherwise when we do not want partition, it still executes. by sff
#if options.part == 0:
#    print "Error: default LLC(L3) partition is 0, we should specify this args '--part=PART'.\n"
#    sys.exit(1)

multiprocesses = []
numThreads = 1
#主要修改部分
if options.benchmark:
    print 'Selected SPEC_CPU2006 4mix benchmark'
    if options.benchmark == 'mix1':
        print '--> bzip2, lbm, gobmk, mcf'
        pro1 = spec06_benchmarks.bzip2
        multiprocesses.append(pro1)
    	pro2 = spec06_benchmarks.bzip2
        multiprocesses.append(pro2)
    	pro3 = spec06_benchmarks.bzip2
        multiprocesses.append(pro3)
    	pro4 = spec06_benchmarks.bzip2
        multiprocesses.append(pro4)
    elif options.benchmark == 'mix2':
    	print '--> gromacs, leslie3d, GemsFDTD, tonto'
        pro1 = spec06_benchmarks.bzip2
        multiprocesses.append(pro1)
    	pro2 = spec06_benchmarks.omnetpp
        multiprocesses.append(pro2)
    	pro3 = spec06_benchmarks.gobmk
        multiprocesses.append(pro3)
    	pro4 = spec06_benchmarks.mcf
        multiprocesses.append(pro4)
    elif options.benchmark == 'mix3':
    	print '-->  perlbench * 4'
        pro1 = spec06_benchmarks.perlbench
        multiprocesses.append(pro1)
    	pro2 = spec06_benchmarks.perlbench
        multiprocesses.append(pro2)
    	pro3 = spec06_benchmarks.perlbench
        multiprocesses.append(pro3)
    	pro4 = spec06_benchmarks.perlbench
        multiprocesses.append(pro4)
    elif options.benchmark == 'mix4':
    	print '-->  mcf * 4'
        pro1 = spec06_benchmarks.mcf
        multiprocesses.append(pro1)
    	pro2 = spec06_benchmarks.mcf
        multiprocesses.append(pro2)
    	pro3 = spec06_benchmarks.mcf
        multiprocesses.append(pro3)
    	pro4 = spec06_benchmarks.mcf
        multiprocesses.append(pro4)
    else:
    	print "No recognized SPEC2006 benchmark selected! Exiting."
    	sys.exit(1)
else:
    print >> sys.stderr, "Need --benchmark switch to specify SPEC CPU2006 workload. Exiting!\n"
    sys.exit(1)

# Set process stdout/stderr
if options.benchmark_stdout:
    process.output = options.benchmark_stdout
    print "Process stdout file: " + process.output
if options.benchmark_stderr:
    process.errout = options.benchmark_stderr
    print "Process stderr file: " + process.errout


(CPUClass, test_mem_mode, FutureClass) = Simulation.setCPUClass(options)
CPUClass.numThreads = numThreads

MemClass = Simulation.setMemClass(options)

# Check -- do not allow SMT with multiple CPUs
if options.smt and options.num_cpus > 1:
    fatal("You cannot use SMT with multiple CPUs!")

np = options.num_cpus
memsize = options.mem_size # sff modify the memory size
memsize = "4096MB"
system = System(cpu = [CPUClass(cpu_id=i) for i in xrange(np)],
                mem_mode = test_mem_mode,
                mem_ranges = [AddrRange(memsize)],
                cache_line_size = options.cacheline_size)

# Create a top-level voltage domain
system.voltage_domain = VoltageDomain(voltage = options.sys_voltage)

# Create a source clock for the system and set the clock period
system.clk_domain = SrcClockDomain(clock =  options.sys_clock,
                                   voltage_domain = system.voltage_domain)

# Create a CPU voltage domain
system.cpu_voltage_domain = VoltageDomain()

# Create a separate clock domain for the CPUs
system.cpu_clk_domain = SrcClockDomain(clock = options.cpu_clock,
                                       voltage_domain =
                                       system.cpu_voltage_domain)

# All cpus belong to a common cpu_clk_domain, therefore running at a common
# frequency.
for cpu in system.cpu:
    cpu.clk_domain = system.cpu_clk_domain

# Sanity check
if options.fastmem:
    if CPUClass != AtomicSimpleCPU:
        fatal("Fastmem can only be used with atomic CPU!")
    if (options.caches or options.l2cache):
        fatal("You cannot use fastmem in combination with caches!")

if options.simpoint_profile:
    if not options.fastmem:
        # Atomic CPU checked with fastmem option already
        fatal("SimPoint generation should be done with atomic cpu and fastmem")
    if np > 1:
        fatal("SimPoint generation not supported with more than one CPUs")

#主要修改部分
for i in xrange(np): 
    system.cpu[i].workload = multiprocesses[i]
    print multiprocesses[i].cmd

    if options.fastmem:
        system.cpu[i].fastmem = True

    if options.simpoint_profile:
        system.cpu[i].simpoint_profile = True
        system.cpu[i].simpoint_interval = options.simpoint_interval


if options.ruby:
    if not (options.cpu_type == "detailed" or options.cpu_type == "timing"):
        print >> sys.stderr, "Ruby requires TimingSimpleCPU or O3CPU!!"
        sys.exit(1)

    # Set the option for physmem so that it is not allocated any space
    system.physmem = MemClass(range=AddrRange(options.mem_size),
                              null = True)

    options.use_map = True
    Ruby.create_system(options, system)
    assert(options.num_cpus == len(system.ruby._cpu_ruby_ports))

    for i in xrange(np):
        ruby_port = system.ruby._cpu_ruby_ports[i]

        # Create the interrupt controller and connect its ports to Ruby
        # Note that the interrupt controller is always present but only
        # in x86 does it have message ports that need to be connected
        system.cpu[i].createInterruptController()

        # Connect the cpu's cache ports to Ruby
        system.cpu[i].icache_port = ruby_port.slave
        system.cpu[i].dcache_port = ruby_port.slave
        if buildEnv['TARGET_ISA'] == 'x86':
            system.cpu[i].interrupts.pio = ruby_port.master
            system.cpu[i].interrupts.int_master = ruby_port.slave
            system.cpu[i].interrupts.int_slave = ruby_port.master
            system.cpu[i].itb.walker.port = ruby_port.slave
            system.cpu[i].dtb.walker.port = ruby_port.slave
else:
    system.membus = SystemXBar()
    system.system_port = system.membus.slave
    CacheConfig_l3.config_cache(options, system)
    MemConfig.config_mem(options, system)

root = Root(full_system = False, system = system)
Simulation.run(options, root, system, FutureClass)
