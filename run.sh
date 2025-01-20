mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ../src && make
# mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug ../src && make

current_path="/home/b09901066/ISPD-NTUEE/NTUGR_v2"
docker_data_path="/docker_data/b09901066"

input_path=$current_path"/input"
output_PR_path=$current_path"/output"
output_log_path=$current_path"/log"

# echo "Running example"
# ./route -def example.def -v example.v.gz -sdc example.sdc -cap $input_path/example.cap -net $input_path/example.net -output $output_PR_path/example.route > $output_log_path/example.log

echo "Running ariane"
./route -cap $input_path/ariane.cap -net $input_path/ariane.net -output $output_PR_path/ariane.route > $output_log_path/ariane.log

# cd ..
# cd evaluation/
# chmod +x evaluation.sh
# bash evaluation.sh