data_list=("ariane")
input_path="/raid/rliang/ISPD25_contest"
output_path="/raid/rliang/ISPD25_contest/results"
for data in "${data_list[@]}"
do
    # run the whole framework
    echo "data: $data"
    ./evaluator $input_path/$data.cap $input_path/$data.net $output_path/$data.route
done