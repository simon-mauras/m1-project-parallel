#! /usr/bin/python3

import os, glob, re, struct

for t in glob.glob("tests/*.env") :
  test = '.'.join(t.split('.')[:-1])
  print("Creation of the binary input file : " + test)
  os.system("./create_input " + test + ".in < " + test + ".env > /dev/null")

for t in glob.glob("tests/*.in") :
  test = '.'.join(t.split('.')[:-1])
  print("---------- " + test + " ----------")
  f = open(t, "rb")
  test_type = struct.unpack('B', f.read(1))[0]
  test_cols = struct.unpack('Q', f.read(8))[0]
  test_rows = struct.unpack('Q', f.read(8))[0]
  f.close()
  print("Simulation...")
  os.system("./run -step 0 -i " + test + ".in -iteration 10000 -dt 1e-2 -grid 1 1 -alldump " + test + "_%05d.dump")
  print("Analysis of the binary output files...")
  mod = 0
  for dump in sorted(glob.glob(test + "_*.dump")) :
    if mod == 0 :
      mod = 30
      base = '.'.join(dump.split('.')[:-1])
      os.system("./display_output " + dump + " " + base + ".pgm " + str(test_cols) + " " + str(test_rows))
    os.system("rm " + dump)
    mod = mod-1
  print("Export...")
  os.system("convert -delay 5 -loop 0 " + test + "_*.pgm " + test + ".gif")
  os.system("rm " + test + "_*.pgm")
  print("Done !")
