mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ../src && make

current_path="/home/b09901066/ISPD-NTUEE/NTUGR_v2"
docker_data_path="/docker_data/b09901066"

input_path=$current_path"/input"
output_PR_path=$current_path"/output"
output_log_path=$current_path"/log"

# echo "Running example"
# ./route -def example.def -v example.v.gz -sdc example.sdc -cap $input_path/example.cap -net $input_path/example.net -output $output_PR_path/example.route > $output_log_path/example.log

echo "Running ariane"
./route -cap $input_path/ariane.cap -net $input_path/ariane.net -output $output_PR_path/ariane.route > $output_log_path/ariane.log

echo "Running bsg_chip"
./route -cap $input_path/bsg_chip.cap -net $input_path/bsg_chip.net -output $output_PR_path/bsg_chip.route > $output_log_path/bsg_chip.log

echo "Running NV_NVDLA_partition_c"
./route -cap $input_path/NV_NVDLA_partition_c.cap -net $input_path/NV_NVDLA_partition_c.net -output $output_PR_path/NV_NVDLA_partition_c.route > $output_log_path/NV_NVDLA_partition_c.log

# cd ..
# cd evaluation/
# chmod +x evaluation.sh
# bash evaluation.sh