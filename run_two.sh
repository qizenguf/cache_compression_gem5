num=$2
default=1000000
sum=$(($num * $default))
num=$3
sum2=$(($num2 * $default))
echo $sum
./run_gem5_alpha_spec06_benchmark_car.sh $1 /home/qizeng/gem5/test3 $sum $sum2& ./run_gem5_alpha_spec06_benchmark_lru.sh $1 /home/qizeng/gem5/test3 $sum $sum2
