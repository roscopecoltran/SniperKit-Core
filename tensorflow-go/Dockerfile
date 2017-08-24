From ubuntu
MAINTAINER Pengfei Ni <feiskyer@gmail.com>

# env for golang
ENV GOPATH /gopath
ENV PATH /${GOPATH}/bin:/usr/local/bin:/usr/local/go/bin/:/usr/local/sbin:/usr/sbin:/usr/bin:/sbin:/bin

# env for tensorflow
ENV TF_NEED_GCP 1
ENV TF_NEED_HDFS 0
ENV TF_NEED_OPENCL 0
ENV TF_NEED_CUDA 0
ENV USE_DEFAULT_PYTHON_LIB_PATH 1
ENV PYTHON_BIN_PATH /usr/bin/python
ENV LD_LIBRARY_PATH ${GOPATH}/src/github.com/tensorflow/tensorflow/bazel-bin/tensorflow

# install development tools
RUN apt-get update && apt-get install -y python swig python-numpy curl git build-essential  && \
	apt-get install -y vim libcurl3-dev software-properties-common && \
	echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
	curl https://bazel.build/bazel-release.pub.gpg |  apt-key add - && \
	add-apt-repository -y ppa:webupd8team/java && apt-get update && \
	echo "oracle-java8-installer shared/accepted-oracle-license-v1-1 select true" | debconf-set-selections && \
	apt-get install -y oracle-java8-installer && \
	apt-get install -y bazel && \
	apt-get clean && \
	curl -sL https://storage.googleapis.com/golang/go1.7.3.linux-amd64.tar.gz | tar -C /usr/local -zxf -

RUN go get -d github.com/tensorflow/tensorflow/tensorflow/go && \
	cd ${GOPATH}/src/github.com/tensorflow/tensorflow && \
	./configure && bazel build -c opt //tensorflow:libtensorflow.so && \
	go generate github.com/tensorflow/tensorflow/tensorflow/go/op

# test whether it is working.
# RUN go test -v github.com/tensorflow/tensorflow/tensorflow/go

WORKDIR ${GOPATH}/src/github.com/tensorflow/tensorflow

