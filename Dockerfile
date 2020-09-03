ARG git_username

# our local base image
FROM ubuntu
LABEL description="Container for use with Visual Studio" 

ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install build dependencies 
RUN apt-get update && apt-get install -y g++ rsync zip openssh-server make git vim

# configure SSH for communication with Visual Studio 
RUN mkdir -p /home/dev/sisop2
RUN cd /home/dev/sisop2 && git clone https://github.com/Andy9822/Instant-Messenger-Cpp
RUN cd /home/dev/sisop2/Instant-Messenger-Cpp && git config user.name "${git_username}"

# docker build --build-arg some_variable_name=a_value
