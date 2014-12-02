rm -rf h_val*
g++ -o sim sim.cpp sim_cache.cpp

./sim 16 4 0 0 0 0 0 val_gcc_trace_mem.txt > h_val_1.txt
diff -iw h_val_1.txt val_1.txt

./sim 32 16 0 0 0 0 0 val_perl_trace_mem.txt > h_val_2.txt
diff -iw h_val_2.txt val_2.txt

./sim 16 4 32 2048 8 0 0 val_gcc_trace_mem.txt > h_val_extra_1.txt
diff -iw h_val_extra_1.txt val_extra_1.txt

./sim 32 8 32 1024 4 2048 8 val_perl_trace_mem.txt > h_val_extra_2.txt
diff -iw h_val_extra_2.txt val_extra_2.txt