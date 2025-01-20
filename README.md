# NTUGR_v2

NTUGR_v2 is a high-performance global router developed for the **ISPD25 Performance-Driven Large Scale Global Routing Contest**. This repository aims to implement cutting-edge routing algorithms leveraging GPU acceleration and machine learning to tackle large-scale global routing challenges.

---

## Features

- **Scalable Design**: Handles benchmarks with up to 50 million cells.
- **Performance-Driven Optimization**:
  - Timing-aware routing with a focus on minimizing WNS and TNS.
  - Power-aware optimizations for reduced energy consumption.
- **GPU Acceleration**: Utilizes CUDA for high-performance pathfinding.
- **Machine Learning Integration**: Explores ML models for congestion estimation and resource allocation.

---

## Prerequisites

To set up this project, ensure the following are installed on your system:
- Docker (Recommended)
- Python 3.8 or higher
- GCC 11 or higher
- CUDA Toolkit (for GPU acceleration)

---

## Docker Environment Setup

### 1. Build the Docker Image
Run the following command in the directory containing the `Dockerfile`:

```bash
docker build -t ispd25 .
```
### 2. Run the Docker Container
Start the container with an interactive shell (with GPU support):
```bash
docker run -it --gpus all --name ispd25_container ispd25
```
If the docker container is already running, you can attach to it:
```bash
docker exec -it ispd25_container bash
```

If the container is not running, you can start it with:
```bash
docker start -i ispd25_container
```

### 3. Verify Docker Installation (Optional)
Check if the Docker container is running successfully (from a separate terminal):
```bash
docker ps
```
You should see the `ispd25` container listed.

To see all images on your system:
```bash
docker images
```

How to stop the container:
```bash
docker stop ispd25_container
docker rm ispd25_container
```

---

## Usage

### 1. Build and Run the Project
To build the project, run the following commands:
```bash
bash run.sh
```
This script compiles the source code and runs the global router on the provided benchmarks.

The command-line arguments for the global router are as follows:
```bash
./route -def ${design}.def -v ${design}.v.gz -sdc ${design}.sdc -cap ${design}.cap -net ${design}.net -output ${design}.route
```

Note that in this repository, we use simplified input files (.cap, .net) as provided in the ISPD25 contest. We don't use .def, .v, and .sdc files.

### 2. Use Docker for Development
To use the Docker container for development, you can mount the project directory to the container:
```bash
docker run -it --rm \
  --gpus all \
  -v /home/b09901066/ISPD-NTUEE/NTUGR_v2:/app/NTUGR_v2 \
  --name ispd25_container \
  ispd25 bash
```

Alternatively, you can use the extension in Visual Studio Code to develop inside the Docker container.
See the tutorial [here](https://medium.com/%E5%A4%BE%E7%B8%AB%E4%B8%AD%E6%B1%82%E7%94%9F%E5%AD%98%E7%9A%84%E4%BA%BA%E9%A1%9E/%E4%BD%BF%E7%94%A8visual-studio-code-%E9%81%A0%E7%AB%AF%E6%93%8D%E4%BD%9Cdocker%E7%92%B0%E5%A2%83%E4%B8%8B%E7%9A%84%E6%AA%94%E6%A1%88-ebb35292a5b1).

### 3. Submitting Results
Use Docker to package the results for submission:

Outside the container:
```bash
docker cp build/route ispd25_container:/workspace/router/route
docker commit ispd25_container ispd_alpha
```

```bash
docker login
docker tag ispd_alpha alanhsiu/ntugr_alpha
docker push alanhsiu/ntugr_alpha
```
