import os, glob, re
for t in glob.glob("tests/*.in") :
  test = '.'.join(t.split('.')[:-1])
  print("---------- " + test + " ----------")
  print("Creation of the binary input file...")
  os.system("./create_input " + test + ".env < " + test + ".in > /dev/null")
  print("Simulation...")
  os.system("./run -step 0 -i " + test + ".env -iteration 10000 -dt 1 -grid 1 1 -alldump " + test + "_%05d.dump")
  print("Analysis of the binary output files...")
  mod = 0
  for dump in sorted(glob.glob(test + "_*.dump")) :
    if mod == 0 :
      mod = 30
      base = '.'.join(dump.split('.')[:-1])
      os.system("./display_output " + dump + " " + base + ".pgm 100 100")
    os.system("rm " + dump)
    mod = mod-1
  print("Export...")
  os.system("convert -delay 5 -loop 0 " + test + "_*.pgm " + test + ".gif")
  os.system("rm " + test + "_*.pgm")
  print("Done !")
