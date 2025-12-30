# if { [ info exists env(CMSSW_BASE) ] } { set CMSSW_BASE $env(CMSSW_BASE) } { set CMSSW_BASE ../../$env(CMSSW_VERSION) }

# open the project
open_project -reset PuppiMETxy
set_top puppimet_xy
add_files firmware/puppimet.cpp -cflags "-std=c++14"

# reset the solution
open_solution -reset "solution"
##   VCU118 dev kit (VU9P)
set_part {xcvu13p-flga2577-2-e}
create_clock -period 2.3
# set_clock_uncertainty 0.2

# synthethize the algorithm
csynth_design

# make ipbb structure and .dep file
file mkdir PuppiMETxy/firmware/hdl;
file mkdir PuppiMETxy/firmware/cfg;
set f [open PuppiMETxy/firmware/cfg/PuppiMETxy.dep "a"];

foreach filepath [glob -dir PuppiMETxy/solution/impl/vhdl/ *] {
  set filename [file tail $filepath];
  file link ./PuppiMETxy/firmware/hdl/$filename ../../solution/impl/vhdl/$filename;
  puts $f "src -l PuppiMETxy $filename"
}
close $f

exit