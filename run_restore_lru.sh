num=$2
default=1000000
sum=$(($num * $default))
num2=$3
sum2=$(($num2 * $default))
echo $sum
sh /home/qizeng/gem5/run_gem5_alpha_spec06_benchmark_lru_cp.sh $1 /home/qizeng/gem5/rest $sum $sum2
