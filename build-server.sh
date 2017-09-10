g++ \
-ggdb \
-L./ \
-Wl,-rpath=./ \
-Wall \
-Werror \
-o server \
netcode.c \
reliable.c \
./tlsf/tlsf.c \
yojimbo.cpp \
server.cpp \
-lsodium \
-lmbedtls \
-lmbedx509 \
-lmbedcrypto \
-lm \
