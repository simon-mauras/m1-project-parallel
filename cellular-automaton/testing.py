import os, glob, re
os.system("./create_input tests/default.env < tests/default.in > /dev/null")
os.system("./run -step 0 -i tests/default.env -iteration 10000 -dt 1 -grid 1 1 -alldump tests/default_%05d.dump")
mod = 0
for dump in sorted(glob.glob("tests/default_*.dump")) :
  if mod == 0 :
    mod = 30
    base = '.'.join(dump.split('.')[:-1])
    os.system("./display_output " + dump + " " + base + ".pgm 100 100")
  mod = mod-1
os.system("convert -delay 5 -loop 0 tests/default_*.pgm tests/default.gif")
os.system("rm tests/default_*.pgm tests/default_*.dump")
