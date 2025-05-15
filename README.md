# N2HE Docker PPFV Demo2

## Requirements 
- server with Docker == 28.0.4 installed

## Installation 
Follow the steps in [Docker Installation](https://docs.docker.com/get-started/get-docker/) for installation of Docker Engine on cloud server. 
Run the command below to ensure that Docker has been installed successfully
```
docker version
``` 


1. Copy all files and folders from this repository to the root directory of server

2. Run command to build the docker images and run the respective docker containers 
```
docker compose up --build  -d
```

3. To shutdown the containers, run command 
```
docker compose down
```

4. To restart the containers 
```
docker compose up -d
```
