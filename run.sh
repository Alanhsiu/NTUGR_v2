mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ../src && make

input_path="/docker_data/b09901066/input"
output_PR_path="/docker_data/b09901066/output"
output_log_path="/home/b09901066/ISPD-NTUEE/NTUGR/output"
echo "Running mempool_tile"
./route -cap $input_path/mempool_tile.cap -net $input_path/mempool_tile.net -output $output_PR_path/mempool_tile.PR_output > $output_log_path/mempool_tile.log

cd ..
cd benchmark/evaluation_script
bash evaluation.sh