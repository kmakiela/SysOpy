SET(SRC calc.c)

add_executable(Server1 server.c ${SRC})
add_executable(Client1 client.c ${SRC})
