g++ -c lib_nfs.cpp -o lib_nfs.o
ar rcs lib_nfs.a lib_nfs.o
g++ -c test3.cpp -o test3.o
g++ -o test3 test3.o -L. -l_nfs
g++ -c server.cpp -o server.o
g++ -o server server.o -L. -l_nfs
