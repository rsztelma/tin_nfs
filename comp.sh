g++ -c lib_nfs.cpp -o lib_nfs.o
ar rcs lib_nfs.a lib_nfs.o
g++ -c test1.cpp -o test1.o
g++ -o test1 test1.o -L. -l_nfs
g++ -c test2.cpp -o test2.o
g++ -o test2 test2.o -L. -l_nfs
g++ -c test3.cpp -o test3.o
g++ -o test3 test3.o -L. -l_nfs
g++ -c test4.cpp -o test4.o
g++ -o test4 test4.o -L. -l_nfs
g++ -c test5.cpp -o test5.o
g++ -o test5 test5.o -L. -l_nfs
g++ -c server.cpp -o server.o
g++ -o server server.o -L. -l_nfs
