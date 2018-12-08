#set yrange [1:]
#set logscale y

#plot "312.log" using 1:2 with boxes, \
#     "123.log" using 1:2 with boxes
set xrange [0:600];

#plot "312.tally" using 1:2 with boxes,\
#     "123.tally" using 1:2 with boxes;

plot "kis3/123.tally" using 1:2 with lines,\
     "kis3/132.tally" using 1:2 with lines,\
     "kis3/213.tally" using 1:2 with lines,\
     "kis3/231.tally" using 1:2 with lines,\
     "kis3/312.tally" using 1:2 with lines,\
     "kis3/321.tally" using 1:2 with lines;

pause 1
reread
