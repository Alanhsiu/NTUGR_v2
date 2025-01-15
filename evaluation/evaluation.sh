data_list=("example")

# root_path is the previous directory of the current directory
root_path=$(dirname $(pwd))
input_path=$root_path"/input"
output_path=$root_path"/output"

for data in "${data_list[@]}"
do
    # run the whole framework
    echo "data: $data"
    ./evaluator $input_path/$data.cap $input_path/$data.net $output_path/$data.route
done