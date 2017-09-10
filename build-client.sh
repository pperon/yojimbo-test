g++ \
-ggdb \
-L./ \
-Wl,-rpath=./ \
-Wall \
-Werror \
-o client \
netcode.c \
reliable.c \
./tlsf/tlsf.c \
yojimbo.cpp \
client.cpp \
-lsodium \
-lmbedtls \
-lmbedx509 \
-lmbedcrypto \
-lm \
