SET(SRC calc.c)

add_executable(Server2 server.c ${SRC})
add_executable(Client2 client.c ${SRC})
