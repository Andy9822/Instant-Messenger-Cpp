# Instant-Messenger-Cpp
The purpose of this project is to implement an instant messages service for synchronous communication between users.


# Dev environment

Docker container with Ubuntu and the necessary tools for compiling C++. 

## Pull the image

- Run `docker pull ubuntu` 
This will download the latest image of Ubuntu from Docker. You can see a list of your docker images by running:

- Run `docker images`
You will be able to confirm if you have the latest version of Ubuntu image.

## Build the container

- `docker build --build-arg git_username=gabriel-inf -t ubuntu-sisop2 .`
By passing your github username it will set the configuration for your account. This argument is mandatory for the build

## Running the container

`docker run --name sisop2 -td ubuntu-sisop2`
This will run the container detached and you will be able to start and stop it calling `docker start/stop sisop2`.

## Install VSCode Remote Development (ms-vscode-remote.vscode-remote-extensionpack)

You can use it to access the folder where you will develop your tasks. 


You will be able to find the project in `/home/dev/sisop2/Instant-Messenger-Cpp` inside the container.