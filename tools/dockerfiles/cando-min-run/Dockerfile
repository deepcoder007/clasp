FROM clasp-min-install
MAINTAINER Christian Schafmeister <meister@temple.edu>

ADD ./extensions/cando/src/lisp clasp/Contents/Resources/source-code/extensions/cando/src/lisp

#RUN cd ~/clasp && git pull origin testing --rebase
#RUN cd ~/clasp/extensions/cando && git pull origin dev --rebase

RUN git clone https://github.com/slime/slime ~/slime
RUN cd /home/app/quicklisp/local-projects && git clone https://github.com/clasp-developers/usocket.git
RUN cd /home/app/quicklisp/local-projects && git clone https://github.com/clasp-developers/cl-jupyter.git
RUN cd /home/app/quicklisp/local-projects && git clone https://github.com/clasp-developers/bordeaux-threads.git
RUN cd /home/app/quicklisp/local-projects && git clone https://github.com/clasp-developers/cffi.git

RUN [ "/home/app/clasp/bin/clasp", "-f", "setup-cando", \
     				 "-e", "(load \"~/quicklisp/setup.lisp\")", \
      				 "-e", "(ql:quickload :trivial-http)", \
      				 "-e", "(require :inet)", \
       				 "-e", "(setq core::*swank-home* \"/home/app/slime\")", \
				 "-e", "(load (format nil \"~a/swank-loader.lisp\" core::*swank-home*))", \
				 "-e", "(swank-loader:init :delete nil :reload nil :load-contribs nil)", \
				 "-e", "(quit)"]


USER root
RUN apt-get -y install python2.7 python-pip python-dev
RUN apt-get -y install ipython ipython-notebook
RUN pip install jupyter
ADD tools/dockerfiles/jupyter/kernel.json /home/app/.local/share/jupyter/kernels/lisp/kernel.json
RUN chown -R app /home/app/.local
USER app
RUN [ "/home/app/clasp/bin/clasp", \
     				 "-e", "(load \"~/quicklisp/setup.lisp\")", \
      				 "-e", "(ql:quickload :cl-jupyter)", \
                                 "-e", "(ql:quickload :pzmq)", \
                                 "-e", "(ql:quickload :cffi)", \
                                 "-e", "(ql:quickload :cffi-grovel)", \
                                 "-e", "(ql:quickload :nibbles)", \
                                 "-e", "(ql:quickload :ironclad)", \
                                 "-e", "(ql:quickload :uuid)", \
                                 "-e", "(ql:quickload :trivial-utf-8)", \
 				 "-e", "(quit)"]

#CMD [ "/home/app/clasp/bin/clasp", "-f", "docker", \
#      				 "-e", "(setq core::*swank-home* \"/home/app/slime\")", \
#      				 "-e", "(load \"/home/app/clasp/src/lisp/modules/docker-swank/start-swank.lisp\")"]

CMD [ "jupyter", "notebook", "--ip=0.0.0.0", "--port=8888", "--no-browser" ]

