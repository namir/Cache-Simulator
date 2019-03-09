# CacheSim
Simulation


# How to compile and run the simulator?

# Step#1: Go to src directory.
cd src

# Step#2: Compile the simulator
make

# Stre#3: Run the simulator from src direcroty
./cachesim ../confs/2way-nwa.conf ../traces/gcc.trace

# If you want to use different configuration and trace file, just change the 
# file names in the ./cahesim command above.

# In order to get Simpulator Output with .out extension whose prefix is based on the name of the input trace file, use
./cachesim ../confs/2way-nwa.conf ../traces/gcc.trace
# from src directory assuming config and trace files are under confs/ and traces/ directory respectively.
# .tout extension will contain the sinulaiton verification data.

# src/runAll.bash will generate all test output file *.tout but only generat gcc.trace.out, gzip.trace.out, mcf.trace.out, #swim.trace.out, twolf.trace.out by over writing output for each trace file. 
# so if you want cc.trace.out, gzip.trace.out, mcf.trace.out, swim.trace.out, twolf.trace.out for each trace then
# run simulation command above instead of src/runAll.bash.
