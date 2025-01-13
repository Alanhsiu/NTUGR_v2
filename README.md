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