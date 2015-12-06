#! /usr/bin/python3

import os, sys, glob, struct

def create(t) :
  test = '.'.join(t.split('.')[:-1])
  print("Creation of the binary input file : " + test)
  os.system("./create_input " + test + ".in < " + test + ".env > /dev/null")

def test(t) :
  test = '.'.join(t.split('.')[:-1])
  print("---------- " + test + " ----------")
  f = open(t, "rb")
  test_type = struct.unpack('B', f.read(1))[0]
  test_rows = struct.unpack('Q', f.read(8))[0]
  test_cols = struct.unpack('Q', f.read(8))[0]
  f.close()
  print("Simulation...")
  os.system("time ./run -step 0 -i " + test + ".in -iteration 10000 -dt 1e-2 -grid 1 1 -alldump " + test + "_%05d.dump -sensor " + test + ".log")
  #os.system("time mpirun -n 1 ./run -step 1 -i " + test + ".in -iteration 5000 -dt 1e-2 -grid 1 1 -lastdump " + test + "_last.dump -sensor " + test + ".log")
  print("Analysis of the binary output files...")
  mod = 0
  for dump in sorted(glob.glob(test + "_*.dump")) :
    if mod == 0 :
      mod = 10
      base = '.'.join(dump.split('.')[:-1])
      os.system("./display_output " + dump + " " + base + ".pgm " + str(test_rows) + " " + str(test_cols))
    os.system("rm " + dump)
    mod = mod-1
  print("Export...")
  
  os.system("convert -delay 5 -loop 0 " + test + "_*.pgm " + test + ".mp4")
  #os.system("convert -delay 5 -loop 0 " + test + "_last.pgm " + test + ".jpg")
  os.system("rm " + test + "_*.pgm")
  print("Done !")


if len(sys.argv) >= 2 :
  for t in sys.argv[1:] : test(t)
else :
  for t in glob.glob("tests/*.env") : create(t)
  for t in glob.glob("tests/*.in") :  test(t)
