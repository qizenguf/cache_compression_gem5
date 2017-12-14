#!/bin/sh
#SBATCH --job-name=compile_gem5_test    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=1
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=2                    # Run on a single CPU
#SBATCH --mem=4080mb                   # Memory limit
#SBATCH --time=272:05:00               # Time limit hrs:min:sec
#SBATCH --output=cp_test_%j.out   # Standard output and error log
 
pwd; hostname; date
 
module load gcc python libz scons swig
export LIBRARY_PATH=/apps/gcc/5.2.0/python/2.7.10/lib:$LIBRARY_PATH
 
#echo "Running plot script on a single CPU core

var1=$1
var2=$2
scons build/ALPHA/gem5.opt -j9
#/home/qizeng/gem5/cp_gem5.sh $1 $2
date
