#set yrange [1:]
#set logscale y

#plot "312.log" using 1:2 with boxes, \
#     "123.log" using 1:2 with boxes

plot "1423.log" using 1:2 with boxes

pause 1
reread
