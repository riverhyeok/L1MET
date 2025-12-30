# open the project
open_project -reset PxPyToPtPhi
set_top pxpy_to_ptphi
add_files firmware/puppimet.cpp -cflags "-std=c++14"

# reset the solution
open_solution -reset "solution"
##   VCU118 dev kit (VU9P)
set_part {xcvu13p-flga2577-2-e}
create_clock -period 2.7
set_clock_uncertainty 0.6

# synthethize the algorithm
csynth_design

# make ipbb structure and .dep file
file mkdir PxPyToPtPhi/firmware/hdl;
file mkdir PxPyToPtPhi/firmware/cfg;
set f [open PxPyToPtPhi/firmware/cfg/PxPyToPtPhi.dep "a"];


foreach filepath [glob -dir PxPyToPtPhi/solution/impl/vhdl/ *] {
  set filename [file tail $filepath];
  file link ./PxPyToPtPhi/firmware/hdl/$filename ../../solution/impl/vhdl/$filename;
  puts $f "src -l PxPyToPtPhi $filename"
}
close $f


exit