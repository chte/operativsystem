set xlabel "Epochs"
set ylabel "Memory allocation"
set terminal png
set output "tstbestcase_memory.png"

plot  "tstincrmalloc0.dat" using 1:2 with linespoints title "Sys malloc",\
      "tstincrmalloc1.dat" using 1:2 with linespoints title "First fit",\
      "tstincrmalloc2.dat" using 1:2 with linespoints title "Best fit",\
